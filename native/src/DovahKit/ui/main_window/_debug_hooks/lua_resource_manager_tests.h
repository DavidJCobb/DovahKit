#pragma once
#include <QMetaType>
#include <QStyledItemDelegate>
#include <QWidget>

//
// GOAL:
// 
// Create a system whereby the Lua VM can manage raster resources (and potentially other resources) in use by 
// multiple widgets and possibly model items. The resources must be stored within the VM so that we control 
// their lifetimes; we must be able to track whether the resources are in use by Qt systems; and we ideally 
// should be able to live-update any widget renders that use the resources.
// 
// The system below allows us to manage resource lifetimes and track resource usage by Qt using smart pointers; 
// these can be stored in QVariants, for model items, and custom widgets can use them directly without wrapping 
// them in QVariants.
// 
// Live-updating widgets is... harder. See the code comments on the test QStyledItemDelegate subclass below for 
// further information.
//

namespace DovahKitDebug {
   class DovahKitTESTQVariantWrappedSmartPointerReferent : public QObject {
      Q_OBJECT;
      public:
         int refcount = 0;
         int value    = 0;

         DovahKitTESTQVariantWrappedSmartPointerReferent(QObject* parent = nullptr) : QObject(parent) {}
         ~DovahKitTESTQVariantWrappedSmartPointerReferent() {
            #if _DEBUG
               //__debugbreak(); // Verify proper destruction.
            #endif
         }

         QList<QPersistentModelIndex> qpmi;
   };
   
   class DovahKitTESTQVariantWrappedSmartPointerReferentRegistry {
      using referent_t = DovahKitTESTQVariantWrappedSmartPointerReferent;
      public:
         static DovahKitTESTQVariantWrappedSmartPointerReferentRegistry& get() {
            static DovahKitTESTQVariantWrappedSmartPointerReferentRegistry instance;
            return instance;
         }
         
      protected:
         QVector<referent_t*> _referents;

      public:
         referent_t* createReferent() {
            auto* r = new referent_t;
            this->_referents.push_back(r);
            return r;
         }
         void destroyReferent(referent_t* r) {
            assert(r->refcount == 0);
            this->_referents.removeAll(r);
            delete r;
         }
         void updateAllReferents() {
            for (auto* r : this->_referents) {
               assert(r);
               for (auto& qpmi : r->qpmi) {
                  if (!qpmi.isValid())
                     continue;
                  auto* model = const_cast<QAbstractItemModel*>(qpmi.model()); // strip const; the change we make here should not invalidate the model
                  if (!model)
                     continue;
                  emit model->dataChanged(qpmi, qpmi, { Qt::ItemDataRole::DecorationRole });
               }
            }
         }
   };


   //
   // TEST: Does QVariant properly manage contained smart pointers? This smart pointer doesn't 
   //       do any deletions; it just manages an intrusive refcount.
   // 
   // RESULT: QVariant does indeed properly handle smart pointer types, so long as they use both 
   //         of the needed QMetaType macros AND have the metatype instantiated at run-time.
   //
   template<class T> requires std::is_base_of_v<QObject, T> class DovahKitTESTQVariantWrappedSmartPointer {
      public:
         using target_t = T;
      protected:
         target_t* target = nullptr;
         QPersistentModelIndex qmi;

         inline void _inc() {
            if (target) {
               ++target->refcount;
               if (qmi.isValid())
                  target->qpmi.push_back(qmi);
            }
         }
         inline void _dec() {
            if (target) {
               if (qmi.isValid())
                  target->qpmi.removeOne(qmi);
               if (--target->refcount == 0)
                  DovahKitTESTQVariantWrappedSmartPointerReferentRegistry::get().destroyReferent(target);
            }
         }

      public:
         DovahKitTESTQVariantWrappedSmartPointer() {}
         DovahKitTESTQVariantWrappedSmartPointer(target_t* v, QModelIndex qmi = QModelIndex()) : target(v), qmi(qmi) {
            this->_inc();
         }
         DovahKitTESTQVariantWrappedSmartPointer(const DovahKitTESTQVariantWrappedSmartPointer& other) {
            this->target = other.target;
            this->qmi    = other.qmi;
            this->_inc();
         }
         DovahKitTESTQVariantWrappedSmartPointer(DovahKitTESTQVariantWrappedSmartPointer&& other) {
            this->target = other.target;
            this->qmi    = other.qmi;
            other.target = nullptr;
            other.qmi    = QModelIndex();
         }
         ~DovahKitTESTQVariantWrappedSmartPointer() {
            this->_dec();
            this->target = nullptr;
            this->qmi    = QModelIndex();
         }

         operator bool() { return this->target != nullptr; };
         operator target_t*() const noexcept { return this->target; };
         target_t* operator->() const noexcept { return this->target; };

         inline QModelIndex index() const noexcept { return this->qmi; }

         DovahKitTESTQVariantWrappedSmartPointer& operator=(target_t* v) noexcept {
            this->_dec();
            this->target = v;
            this->qmi    = QModelIndex();
            this->_inc();
            return *this;
         }
         DovahKitTESTQVariantWrappedSmartPointer& operator=(const DovahKitTESTQVariantWrappedSmartPointer& other) noexcept {
            this->_dec();
            this->target = other.target;
            this->qmi    = other.qmi;
            this->_inc();
            return *this;
         }
         DovahKitTESTQVariantWrappedSmartPointer& operator=(DovahKitTESTQVariantWrappedSmartPointer&& other) noexcept {
            this->_dec();
            this->target = other.target;
            this->qmi    = other.qmi;
            other.target = nullptr;
            other.qmi    = QModelIndex();
            return *this;
         }
   };

   //
   // TEST: Can we use a styled item delegate to have Qt::DecorationRole render from an outside resource? If 
   //       so, does updating that resource "from afar" live-update the rendered widget?
   // 
   // RESULT: The delegate is only invoked when the widget wants to repaint an item, and the widget will try 
   //         to repaint items only when it thinks they are likely to have changed in some way -- in practice, 
   //         when the cursor enters or leaves either an individual item or the entire view. Live updates to 
   //         the data that our delegate fetches (e.g. on a timer) have no effect.
   // 
   //         It seems that the only way to force the widget to repaint would be to actually reach out and 
   //         tell it to update. If we design custom widgets which use LuaManagedResources, we can simply 
   //         implement our own signals and slots to effect this. For model items, things get a bit uglier: 
   //         every LuaManagedResource needs to retain a list of QPersistentModelIndexes pointing to each 
   //         item using the resource, so that we can emit dataChanged signals on those whenever the resource 
   //         data is modified; this in turn means that large models that use lots of icons (e.g. every cell 
   //         in a column has an icon) will have large numbers of QPMIs, all of which must be updated one by 
   //         one with every change to the model layout even if we never actually do modify the resource data 
   //         remotely.
   // 
   //         (Not to mention: QPMIs actually aren't sufficient. We need model observers, at least if we want 
   //         to be able to define default icons for rows and columns. And we do want that.)
   // 
   //         Alternatively, if we have the VM keep track of what widgets are using what models, we can have 
   //         our smart-pointer class just notify the VM when models as a whole gain or lose references to 
   //         the resource; then, we can force updates on individual widgets. Refer to uses of the constexpr 
   //         (update_entire_widgets) bool in the CPP file. If we do it this way, then we don't need to make 
   //         a QPMI for every model item that refers to a resource, which means that there'll be overhead 
   //         on modifying resources that are being displayed (we have to track every model that refers to 
   //         a resource, and then ask the VM to update all widgets for these models), but there won't be 
   //         overhead on modifying a model that happens to use resources. I think that's the better way to 
   //         do things.
   //
   class DovahKitTESTQStyledItemDelegate : public QStyledItemDelegate {
      protected:
         static DovahKitTESTQVariantWrappedSmartPointerReferent* _extract_referent(const QModelIndex& index);
      public:
         DovahKitTESTQStyledItemDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {};

         // This by itself is not sufficient for real-time updates.
         virtual void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;

         // This does not allow us to do real-time updates either. I'd tried copying the (option) and modifying 
         // the copy before then passing it into a call-super; that simply didn't work.
         //virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
   };

   extern void run_lua_resource_manager_tests(QWidget* parent);
}

// These macros don't work from within a namespace. Ignore IntelliSense errors on them, too; those may be false-positives.
Q_DECLARE_SMART_POINTER_METATYPE(DovahKitDebug::DovahKitTESTQVariantWrappedSmartPointer);
Q_DECLARE_METATYPE(DovahKitDebug::DovahKitTESTQVariantWrappedSmartPointer<DovahKitDebug::DovahKitTESTQVariantWrappedSmartPointerReferent>);