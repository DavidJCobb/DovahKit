#include "raster.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/lua_managed_resources.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../editor_script_core.h"
#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"
#include "../../ui/util/color.h"

#include "../../../../helpers/lua/error.h"
#include "../../../../helpers/lua/for_each_in_array.h"
#include "../../../../helpers/lua/istablelike.h"
#include "../../../../helpers/lua/qt_point.h"
#include "../../../../helpers/lua/set_top_on_exit.h"
#include "../../../../helpers/lua/tostringex.h"
#include "../../../../helpers/rotation.h"

#include <QPainter>

namespace {
   using namespace editor_script;
   using cls = wrappers::resource::raster;

   namespace _helpers {
      void pull_fill_color(lua_State* L, int index, QBrush& brush, bool optional = false) {
         index = lua_absindex(L, index);
         if (optional && lua_isnoneornil(L, index)) {
            brush.setColor(QColorConstants::Transparent);
            return;
         }
         std::string error;
         QColor      color = util::ui::protected_pull_color(L, index, error);
         if (!color.isValid()) {
            if (error.empty())
               cobb::lua::error(L, "options.fill_color was not a valid color");
            cobb::lua::error(L, "options.fill_color was not a valid color: %s", error.c_str());
         }
         brush.setColor(color);
      }

      // Given a Lua table (or nil), pull configuration information for a QGradient from it and overwrite the input 
      // brush if valid information is found. This function needs to receive the bounding rect that will be filled 
      // with a gradient so that it can properly set up start and end points for QLinearGradient.
      void pull_fill_gradient(lua_State* L, int index, QBrush& brush, QRectF bound) {
         index = lua_absindex(L, index);
         if (lua_isnoneornil(L, index)) {
            return;
         }
         if (!cobb::lua::istablelike(L, index))
            cobb::lua::error(L, "options.fill_gradient was neither nil nor a table");
         //
         auto top   = lua_gettop(L);
         auto guard = cobb::lua::set_top_on_exit(L, top);
         //
         QGradient::Type type = QGradient::Type::LinearGradient;
         lua_getfield(L, index, "type");
         if (!lua_isnoneornil(L, -1)) {
            std::string tn;
            if (!cobb::lua::tostringex(L, -1, tn))
               cobb::lua::error(L, "options.fill_gradient.type was neither nil nor a string, but was instead type `%s`", lua_typename(L, lua_type(L, -1)));
            //
            type = QGradient::Type::NoGradient;
            constexpr std::array map = {
               std::pair{ QGradient::Type::ConicalGradient, "conical" },
               std::pair{ QGradient::Type::LinearGradient,  "linear" },
               std::pair{ QGradient::Type::RadialGradient,  "radial" },
            };
            for (auto& pair : map) {
               if (stricmp(tn.c_str(), pair.second) == 0) {
                  type = pair.first;
                  break;
               }
            }
            if (type == QGradient::Type::NoGradient)
               cobb::lua::error(L, "options.fill_gradient.type was not a recognized type (\"conical\", \"linear\", or \"radial\") but was instead \"%s\"", tn.c_str());
         }
         lua_pop(L, 1);
         //
         QVector<QGradientStop> stops;
         {
            bool is_sorted = true;
            //
            lua_getfield(L, index, "stops");
            int code = cobb::lua::for_each_in_array(L, -1, [&stops, &is_sorted](lua_State* L, int i) {
               if (!cobb::lua::istablelike(L, -1))
                  cobb::lua::error(L, "options.fill_gradient.stops[%d] was not a table", i);
               lua_geti(L, -1, 1);
               lua_geti(L, -2, 2);
               if (!lua_isnumber(L, -2))
                  cobb::lua::error(L, "options.fill_gradient.stops[%d][1] should've been the stop position (a number between 0 and 1, inclusive) but is not a number", i);
               //
               std::string   error;
               QGradientStop stop;
               stop.first = lua_tonumber(L, -2);
               if (stop.first < 0.0 || stop.first > 1.0)
                  cobb::lua::error(L, "options.fill_gradient.stops[%d][1] is out of bounds; it must be between 0 and 1 inclusive", i);
               stop.second = util::ui::protected_pull_color(L, -1, error);
               if (!stop.second.isValid()) {
                  if (!error.empty())
                     cobb::lua::error(L, "options.fill_gradient.stops[%d][2] is not a valid color: %s", i, error.c_str());
                  cobb::lua::error(L, "options.fill_gradient.stops[%d][2] is not a valid color", i);
               }
               if (is_sorted && i > 1) {
                  if (stop.first < stops.back().first) {
                     is_sorted = false;
                  }
               }
               stops.push_back(stop);
            });
            switch (code) {
               case 0:
                  break; // no error
               case 1:
                  cobb::lua::error(L, "options.fill_gradient.stops has a zero length");
               case -1:
                  cobb::lua::error(L, "options.fill_gradient.stops was not a table");
               case -2:
                  cobb::lua::error(L, "options.fill_gradient.stops has a non-integer length");
               case -3:
                  cobb::lua::error(L, "options.fill_gradient.stops has a negative length");
               default:
                  cobb::lua::error(L, "options.fill_gradient.stops is not a valid array");
            }
            lua_pop(L, 1);
            //
            if (!is_sorted) {
               qStableSort(stops.begin(), stops.end(), [](const QGradientStop& a, const QGradientStop& b) {
                  return a.first < b.first;
               });
            }
         }
         //
         bool has_angle  = false;
         bool has_center = false;
         qreal   angle;
         QPointF center;
         lua_getfield(L, index, "angle");
         lua_getfield(L, index, "center");
         if (lua_isnumber(L, -2)) {
            angle     = cobb::degrees_to_radians(lua_tonumber(L, -2));
            has_angle = true;
         }
         if (cobb::lua::istablelike(L, -1)) {
            lua_getfield(L, -1, "x");
            lua_getfield(L, -2, "y");
            if (lua_isnumber(L, -1) && lua_isnumber(L, -2)) {
               center.setX(lua_tonumber(L, -2));
               center.setY(lua_tonumber(L, -1));
               has_center = true;
            }
            lua_pop(L, 2);
         }
         lua_pop(L, 2);
         //
         QGradient* result = nullptr;
         switch (type) {
            case QGradient::Type::LinearGradient:
               {
                  if (!has_angle)
                     cobb::lua::error(L, "options.fill_gradient is a linear gradient, but options.fill_gradient.angle was not a number");
                  //
                  lua_getfield(L, index, "stretch");
                  if (!lua_isnoneornil(L, -1)) {
                     if (!lua_isboolean(L, -1))
                        cobb::lua::error(L, "options.fill_gradient is a linear gradient, but options.fill_gradient.stretch was neither nil nor a boolean");
                     if (lua_toboolean(L, -1)) {
                        auto w    = bound.width();
                        auto h    = bound.height();
                        auto side = std::max(w, h);
                        bound.setSize({ side, side });
                        bound.translate({ -(side - w) / 2.0, -(side - h) / 2.0 });
                     }
                  }
                  lua_pop(L, 1);
                  //
                  auto* lg = new QLinearGradient;
                  //
                  if (angle) {
                     //
                     // For angles other than 0, 90, 180, 270, and 360, we need to do some geometry in order to fit the 
                     // gradient to the bounds of the drawn object. We want to accept an angle (and maybe, in the future, 
                     // an offset), but Qt wants to take start and end locations, so we need to convert from the one to 
                     // the other.
                     // 
                     auto vx = cos(angle);
                     auto vy = sin(angle);
                     if (!has_center)
                        center = bound.center();
                     else {
                        if (!bound.contains(center))
                           cobb::lua::error(L, "options.fill_gradient is a linear gradient and specified a bad center (point (%f, %f) lies out of bounds)", center.x(), center.y());
                     }
                     //
                     // Per: <https://stackoverflow.com/a/3197924>
                     auto get_intersection = [](double angle, const QRectF& bound, const QPointF from) -> QPointF {
                        double vx = cos(angle);
                        double vy = sin(angle);
                        double t  = NAN;
                        std::array sides = {
                           (bound.left()   - from.x()) / vx, // intersects left
                           (bound.right()  - from.x()) / vx, // intersects right
                           (bound.top()    - from.y()) / vy, // intersects top
                           (bound.bottom() - from.y()) / vy, // intersects bottom
                        };
                        // The smallest positive value of the above four is the value of (t), usable to compute the 
                        // intersection.
                        for (auto v : sides) {
                           if (v > 0) {
                              if (isnan(t))
                                 t = v;
                              else
                                 t = std::min(t, v);
                           }
                        }
                        //
                        assert(!isnan(t));
                        return { from.x() + (t * vx), from.y() + (t * vy) };
                     };
                     //
                     QPointF forward_intersection  = get_intersection(angle,            bound, center);
                     QPointF backward_intersection = get_intersection(angle - cobb::pi, bound, center);
                     //
                     lg->setStart(backward_intersection);
                     lg->setFinalStop(forward_intersection);
                  } else {
                     auto mag = std::max(bound.width(), bound.height());
                     lg->setStart({ 0, 0 });
                     lg->setFinalStop({ cos(angle) * mag, sin(angle) * mag });
                  }
                  //
                  result = lg;
               }
               break;
            case QGradient::Type::ConicalGradient:
               {
                  if (!has_angle)
                     cobb::lua::error(L, "options.fill_gradient is a conical gradient, but options.fill_gradient.angle was not a number");
                  if (!has_center)
                     cobb::lua::error(L, "options.fill_gradient is a conical gradient, but options.fill_gradient.center was not a table or did not have the expected coordinates");
                  auto* cg = new QConicalGradient;
                  cg->setAngle(angle);
                  cg->setCenter(center);
                  result = cg;
               }
               break;
            case QGradient::Type::RadialGradient:
               {
                  if (!has_center)
                     cobb::lua::error(L, "options.fill_gradient is a radial gradient, but options.fill_gradient.center was not a table or did not have the expected coordinates");
                  lua_getfield(L, index, "radius");
                  if (!lua_isnumber(L, -1))
                     cobb::lua::error(L, "options.fill_gradient is a radial gradient, but options.fill_gradient.radius was not a number");
                  qreal radius = lua_tonumber(L, -1);
                  lua_pop(L, 1);
                  if (radius <= 0)
                     cobb::lua::error(L, "options.fill_gradient is a radial gradient, but options.fill_gradient.radius was zero or negative");
                  auto* rg = new QRadialGradient;
                  rg->setCenter(center);
                  rg->setCenterRadius(radius);
                  result = rg;
               }
               break;
         }
         assert(result);
         result->setStops(stops);
         brush = QBrush(*result);
      }

      void pull_line_color(lua_State* L, int index, QPen& pen, bool optional = false) {
         index = lua_absindex(L, index);
         if (optional && lua_isnoneornil(L, index)) {
            pen.setColor(QColorConstants::Transparent);
            return;
         }
         std::string error;
         QColor      color = util::ui::protected_pull_color(L, index, error);
         if (!color.isValid()) {
            if (error.empty())
               luaL_error(L, "options.line_color was not a valid color");
            luaL_error(L, "options.line_color was not a valid color: %s", error.c_str());
         }
         pen.setColor(color);
      }

      void pull_line_width(lua_State* L, int index, QPen& pen) {
         index = lua_absindex(L, index);
         if (lua_isnoneornil(L, index)) {
            pen.setWidthF(1.0F);
            return;
         }
         if (!lua_isnumber(L, index))
            luaL_error(L, "options.line_width was neither nil nor a number");
         qreal width = lua_tonumber(L, index);
         if (width <= 0)
            luaL_error(L, "options.line_width was a number less than or equal to zero");
         pen.setWidthF(width);
      }

      bool pull_qpoint_f(lua_State* L, int index, QPointF& out) {
         index = lua_absindex(L, index);
         if (!cobb::lua::istablelike(L, index))
            return false;
         lua_checkstack(L, 2);
         lua_geti(L, index, 1);
         lua_geti(L, index, 2);
         if (!lua_isnumber(L, -2) || !lua_isnumber(L, -1)) {
            lua_pop(L, 2);
            lua_getfield(L, index, "x");
            lua_getfield(L, index, "y");
            if (!lua_isnumber(L, -2) || !lua_isnumber(L, -1)) {
               lua_pop(L, 2);
               return false;
            }
         }
         out.setX(lua_tonumber(L, -2));
         out.setY(lua_tonumber(L, -1));
         lua_pop(L, 2);
         return true;
      }
   }

   namespace _methods {
      luastackchange_t draw_line(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, cobb::lua::istablelike(L, 2), 2, "table (options) expected");
         lua_settop(L, 2);
         lua_getfield(L, 2, "from");       // 3
         lua_getfield(L, 2, "to");         // 4
         lua_getfield(L, 2, "line_color"); // 5
         lua_getfield(L, 2, "line_width"); // 6
         if (!cobb::lua::istablelike(L, 3)) {
            auto type = lua_type(L, 3);
            luaL_error(L, "options.from must be a table; got %s", lua_typename(L, type));
         }
         if (!cobb::lua::istablelike(L, 4)) {
            auto type = lua_type(L, 4);
            luaL_error(L, "options.to must be a table; got %s", lua_typename(L, type));
         }
         std::array<QPointF, 2> endpoints;
         QPen pen;
         //
         // Get endpoints:
         //
         {
            constexpr std::array names = { "options.from", "options.to" };
            for (int i = 0; i < 2; ++i)
               endpoints[i] = cobb::lua::pull_qpointf(L, 3 + i, names[i]);
         }
         //
         // Get pen settings:
         //
         _helpers::pull_line_color(L, 5, pen);
         _helpers::pull_line_width(L, 6, pen);
         //
         // Begin drawing:
         //
         if (!self.managed_resource)
            return 0;
         self.managed_resource->modify_raster_script_side([endpoints, pen](QImage& image) {
            QPainter painter(&image);
            painter.setPen(pen);
            painter.drawLine(endpoints[0], endpoints[1]);
         });
         return 0;
      }
      luastackchange_t draw_raster(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* arg  = wrapper_from_stack<cls>(L, 2);
         luaL_argcheck(L, arg != nullptr, 2, "raster expected");
         int isnum;
         int x = lua_tointegerx(L, 3, &isnum);
         luaL_argcheck(L, isnum,  3, "integer (x-coordinate) expected");
         luaL_argcheck(L, x != 0, 3, "x-coordinate cannot be zero");
         luaL_argcheck(L, x >= 0, 3, "x-coordinate cannot be negative");
         int y = lua_tointegerx(L, 4, &isnum);
         luaL_argcheck(L, isnum,  4, "integer (y-coordinate) expected");
         luaL_argcheck(L, y != 0, 4, "y-coordinate cannot be zero");
         luaL_argcheck(L, y >= 0, 4, "y-coordinate cannot be negative");
         if (!self.managed_resource)
            return 0;
         auto* other = arg->managed_resource;
         if (!other)
            return 0;
         //
         self.managed_resource->modify_raster_script_side([other, x, y](QImage& image) {
            auto img = other->get_raster_script_side();
            //
            QPainter painter(&image);
            painter.drawImage(QPoint{ x, y }, img);
         });
         //
         return 0;
      }
      luastackchange_t draw_rect(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, cobb::lua::istablelike(L, 2), 2, "table (options) expected");
         lua_settop(L, 2);
         //
         QRectF rect;
         {
            QPointF from;
            QPointF to;
            //
            lua_getfield(L, 2, "from"); // 3
            if (!_helpers::pull_qpoint_f(L, 3, from)) {
               lua_getfield(L, 2, "x"); // 4
               lua_getfield(L, 2, "y"); // 5
               if (!lua_isnumber(L, 4))
                  cobb::lua::error(L, "options.from was unspecified or invalid, and options.x was not a number");
               if (!lua_isnumber(L, 5))
                  cobb::lua::error(L, "options.from was unspecified or invalid, and options.y was not a number");
               from.setX(lua_tonumber(L, 4));
               from.setY(lua_tonumber(L, 5));
               lua_pop(L, 2);
            }
            lua_pop(L, 1);
            rect.setTopLeft(from);
            //
            lua_getfield(L, 2, "to"); // 3
            if (_helpers::pull_qpoint_f(L, 3, to)) {
               rect.setBottomRight(to);
            } else {
               lua_getfield(L, 2, "w"); // 4
               if (!lua_isnumber(L, 4)) {
                  lua_pop(L, 1);
                  lua_getfield(L, 2, "width");
                  if (!lua_isnumber(L, 4))
                     cobb::lua::error(L, "options.to was unspecified or invalid, and neither options.w nor options.width were numbers");
               }
               rect.setWidth(lua_tonumber(L, 4));
               lua_pop(L, 1);
               //
               lua_getfield(L, 2, "h"); // 4
               if (!lua_isnumber(L, 4)) {
                  lua_pop(L, 1);
                  lua_getfield(L, 2, "height");
                  if (!lua_isnumber(L, 4))
                     cobb::lua::error(L, "options.to was unspecified or invalid, and neither options.h nor options.height were numbers");
               }
               rect.setHeight(lua_tonumber(L, 4));
               lua_pop(L, 1);
            }
            lua_pop(L, 1);
         }
         //
         QPen   pen;
         QBrush brush = QBrush(Qt::SolidPattern);
         //
         assert(lua_gettop(L) == 2);
         lua_getfield(L, 2, "line_color");    // 3
         lua_getfield(L, 2, "line_width");    // 4
         lua_getfield(L, 2, "fill_color");    // 5
         lua_getfield(L, 2, "fill_gradient"); // 6
         //
         // Get pen settings:
         //
         _helpers::pull_line_color   (L, 3, pen, true);
         _helpers::pull_line_width   (L, 4, pen);
         _helpers::pull_fill_color   (L, 5, brush, true);
         _helpers::pull_fill_gradient(L, 6, brush, rect);
         if (!brush.gradient() && !brush.color().isValid())
            cobb::lua::error(L, "neither options.fill_color nor options.fill_gradient appear to have been specified");
         //
         // Begin drawing:
         //
         if (!self.managed_resource)
            return 0;
         self.managed_resource->modify_raster_script_side([rect, pen, brush](QImage& image) {
            QPainter painter(&image);
            painter.setPen(pen);
            painter.setBrush(brush);
            painter.drawRect(rect);
         });
         return 0;
      }
      luastackchange_t fill(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         //
         QColor color = util::ui::pull_color(L, 2);
         //
         self.managed_resource->modify_raster_script_side([color](QImage& image) {
            image.fill(color);
         });
         //
         return 0;
      }
      luastackchange_t get_pixel(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         //
         int isnum;
         int x = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum,  2, "x-coordinate (integer) expected");
         luaL_argcheck(L, x != 0, 2, "x-coordinate cannot be zero");
         luaL_argcheck(L, x >= 0, 2, "x-coordinate cannot be negative");
         int y = lua_tointegerx(L, 3, &isnum);
         luaL_argcheck(L, isnum,  3, "y-coordinate (integer) expected");
         luaL_argcheck(L, y != 0, 3, "y-coordinate cannot be zero");
         luaL_argcheck(L, y >= 0, 3, "y-coordinate cannot be negative");
         --x;
         --y;
         //
         auto image = self.managed_resource->get_raster_script_side();
         util::ui::push_color(L, image.pixel(x, y));
         return 1;
      }
      luastackchange_t set_pixel(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         //
         int isnum;
         int x = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum,  2, "x-coordinate (integer) expected");
         luaL_argcheck(L, x != 0, 2, "x-coordinate cannot be zero");
         luaL_argcheck(L, x >= 0, 2, "x-coordinate cannot be negative");
         int y = lua_tointegerx(L, 3, &isnum);
         luaL_argcheck(L, isnum,  3, "y-coordinate (integer) expected");
         luaL_argcheck(L, y != 0, 3, "y-coordinate cannot be zero");
         luaL_argcheck(L, y >= 0, 3, "y-coordinate cannot be negative");
         --x;
         --y;
         QColor color = util::ui::pull_color(L, 4);
         //
         int w = -1;
         int h = -1;
         self.managed_resource->modify_raster_script_side([x, y, color, &w, &h](QImage& image) {
            assert(image.format() == QImage::Format::Format_ARGB32);
            w = image.width();
            h = image.height();
            if (x >= w || y >= h)
               return;
            auto* bytes = (QRgb*)image.scanLine(y);
            bytes[x] = color.rgba();
         });
         luaL_argcheck(L, x < w, 2, "x-coordinate exceeded the raster's width");
         luaL_argcheck(L, y < h, 3, "y-coordinate exceeded the raster's height");
         //
         return 0;
      }
   }
   namespace _getters {
      luastackchange_t height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         auto image = self.managed_resource->get_raster_script_side();
         if (image.isNull())
            return 0;
         lua_pushinteger(L, image.height());
         return 1;
      }
      luastackchange_t width(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         auto image = self.managed_resource->get_raster_script_side();
         if (image.isNull())
            return 0;
         lua_pushinteger(L, image.width());
         return 1;
      }
   }
   namespace _setters {
      /*//
      luastackchange_t auto_default(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QPushButton::setAutoDefault, value);
         return 0;
      }
      luastackchange_t flat(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QPushButton::setFlat, value);
         return 0;
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QPushButton::setText, value);
         return 0;
      }
      //*/
   }

   namespace _singleton_functions {
      luastackchange_t new_(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_ui_permissions();
         //
         int    width      = -1;
         int    height     = -1;
         QColor background = Qt::GlobalColor::transparent;
         lua_settop(L, 1);
         {
            int isnum;
            int type = lua_type(L, 1);
            if (type != LUA_TTABLE && type != LUA_TUSERDATA)
               luaL_error(L, "expected options table or userdata as argument; got %s", lua_typename(L, type));
            lua_getfield(L, 1, "width");
            width = lua_tointegerx(L, 2, &isnum);
            luaL_argcheck(L, isnum, 2, "expected integer width");
            lua_pop(L, 1);
            lua_getfield(L, 1, "height");
            height = lua_tointegerx(L, 2, &isnum);
            luaL_argcheck(L, isnum, 2, "expected integer height");
            lua_pop(L, 1);
            //
            luaL_argcheck(L, width  >= 0, 1, "width cannot be negative");
            luaL_argcheck(L, width  != 0, 1, "width cannot be zero");
            luaL_argcheck(L, height >= 0, 1, "height cannot be negative");
            luaL_argcheck(L, height != 0, 1, "height cannot be zero");
            //
            lua_getfield(L, 1, "background_color");
            if (!lua_isnoneornil(L, 2)) {
               background = util::ui::pull_color(L, 2);
            }
            lua_pop(L, 1);
         }
         //
         LuaManagedResourceHandle resource = nullptr;
         {
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [width, height, background, &resource]() {
               auto base = QImage(width, height, QImage::Format::Format_ARGB32);
               if (base.isNull()) // can occur if the image is too large?
                  return;
               base.fill(background);
               resource = DovahKitScriptVMResourceInterface::get().create_resource(base);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
            //
            if (!resource) {
               luaL_error(L, "unable to create %dx%dpx raster", width, height);
            }
         }
         return cls::wrap_and_push(L, *resource);
      }
      luastackchange_t is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace editor_script::wrappers::resource {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "draw_line",   &_methods::draw_line },
      { "draw_raster", &_methods::draw_raster },
      { "draw_rect",   &_methods::draw_rect },
      { "fill",        &_methods::fill },
      { "get_pixel",   &_methods::get_pixel },
      { "set_pixel",   &_methods::set_pixel },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "height", &_getters::height },
      { "width",  &_getters::width },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
   };

   /*static*/ void cls::setup(lua_State* L) {
      int pos = lua_gettop(L);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
      //
      assert(lua_gettop(L) == pos + 1);
      lua_setglobal(L, cls::global_name);
   }

   /*static*/ int cls::wrap_and_push(lua_State* L, LuaManagedResource& resource) {
      assert(resource.resource_type() == lua_managed_resource_type::raster);
      wrapper out;
      out.type = wrapper_type::lua_managed_resource;
      out.managed_resource = &resource;
      return DovahKitScriptVMUserdataInterface::get().push(L, out, cls::metatable_key);
   }
}