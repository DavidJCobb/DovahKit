#include "raster_draw_path.h"
#include <QPainterPath>
#include <QVector2D>
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/istablelike.h"
#include "../../../helpers/lua/qt_point.h"
#include "../../../helpers/lua/set_top_on_exit.h"
#include "../../../helpers/math.h"
#include "../../../helpers/rotation.h"
#include "../../send_script_task.h"

#include "../../tasks/s2m/lambda.h"

namespace {
   using namespace dovahscript;
   using cls = wrappers::raster_draw_path;

   struct _wrapped_path {
      bool          dead = false;
      QPainterPath* path = nullptr;

      void teardown() {
         if (this->dead)
            return;
         this->dead = true;
         delete this->path;
         this->path = nullptr;
      }

      static _wrapped_path* create(lua_State* L) {
         auto* memory   = lua_newuserdata(L, sizeof(_wrapped_path));
         auto* instance = new (memory) _wrapped_path;
         //
         lua_getfield(L, LUA_REGISTRYINDEX, cls::metatable_key); // push 1
         if (lua_isnoneornil(L, -1)) {
            assert(false && "The wrapper-class wasn't set up properly; its metatable is undefined.");
            cobb::lua::error(L, "internal program error: the metatable for raster_draw_path wasn't set up properly, so instances cannot be created");
            return 0;
         }
         lua_setmetatable(L, -2); // pop 1
         //
         return instance;
      }
      
      static int __close(lua_State* L) {
         auto* userdata = (_wrapped_path*) lua_touserdata(L, 1);
         userdata->teardown();
         return 0;
      }
      static int __gc(lua_State* L) {
         auto* userdata = (_wrapped_path*) lua_touserdata(L, 1);
         userdata->teardown();
         userdata->~_wrapped_path();
         lua_pushnil(L);
         lua_setmetatable(L, 1); // Lua can't guarantee that __gc will only be called once, so make sure there *is* no __gc to call a second time
         return 0;
      }
   };

   namespace _methods {
      int add_ellipse(lua_State* L) {
         auto* path = cls::pull_self(L);
         luaL_argcheck(L, cobb::lua::istablelike(L, 2), 2, "table (options) expected");
         lua_settop(L, 2);
         //
         QPointF center;
         QSizeF  radii;
         //
         lua_getfield(L, 2, "center"); // 3
         if (cobb::lua::pull_qpoint_float(L, 3, center) != 0) {
            lua_getfield(L, 2, "x"); // 4
            lua_getfield(L, 2, "y"); // 5
            if (!lua_isnumber(L, 3))
               cobb::lua::error(L, "options.center was unspecified or invalid, and options.x was not a number");
            if (!lua_isnumber(L, 4))
               cobb::lua::error(L, "options.center was unspecified or invalid, and options.y was not a number");
            center.setX(lua_tonumber(L, 3));
            center.setY(lua_tonumber(L, 4));
            lua_pop(L, 2);
         }
         lua_pop(L, 1);
         //
         lua_getfield(L, 2, "radius"); // 3
         if (!lua_isnoneornil(L, 3)) {
            if (!lua_isnumber(L, 3))
               cobb::lua::error(L, "options.radius was neither nil nor a number");
            auto n = lua_tonumber(L, 3);
            radii.setWidth(n);
            radii.setHeight(n);
         } else {
            QPointF working;
            lua_getfield(L, 2, "radii"); // 4
            if (!cobb::lua::pull_qpoint_float(L, 4, working))
               cobb::lua::error(L, "options.radius was unspecified, and options.radii was unspecified or invalid");
            lua_pop(L, 1);
            radii.setWidth(working.x());
            radii.setHeight(working.y());
         }
         lua_pop(L, 1);
         //
         center += { -1, -1 }; // Lua values should start from (1, 1)
         //
         path->closeSubpath();
         path->addEllipse(center, radii.width(), radii.height());
         path->closeSubpath();
         return 0;
      }
      int arc_to(lua_State* L) {
         auto* path = cls::pull_self(L);
         QPointF from;
         QPointF by;
         QPointF to;
         qreal   radius;
         //
         std::array<int, 2> codes;
         //
         from = path->currentPosition();
         codes[0] = cobb::lua::pull_qpoint_float(L, 2, by);
         codes[1] = cobb::lua::pull_qpoint_float(L, 3, to);
         for (int i = 0; i < codes.size(); ++i) {
            switch (codes[i]) {
               case 0:
                  break;
               case -1:
                  cobb::lua::argerror(L, 2 + i, "table (from-point) expected");
               case -2:
                  cobb::lua::argerror(L, 2 + i, "no x-coordinate specified");
               case -3:
                  cobb::lua::argerror(L, 2 + i, "no y-coordinate specified");
            }
         }
         int isnum;
         radius = lua_tonumberx(L, 4, &isnum);
         luaL_argcheck(L, isnum,      5, "number (radius) expected");
         luaL_argcheck(L, radius > 0, 5, "radius must be non-negative");
         //
         bool flat = radius == 0.0 || from == by || by == to;
         if (!flat) {
            //
            // We can check if the three points are collinear -- that is, if they all fit on the same flat 
            // line -- by checking if they form a triangle with area 0.
            //
            auto a = QVector2D(from - by).length();
            auto b = QVector2D(by   - to).length();
            auto c = QVector2D(to   - from).length();
            //
            auto semiperimeter = (a + b + c) / 2.0;
            auto area_sq = semiperimeter * (semiperimeter - a) * (semiperimeter - b) * (semiperimeter - c);
            //
            flat = area_sq == 0.0;
         }
         if (flat) {
            //
            // Per HTML5, edge-cases default to drawing a straight line from (from) to (by), not to (to). 
            // It's a strange and ugly fallback, but testing shows that browsers honor the spec, so we will 
            // as well.
            //
            path->lineTo(by);
            return 0;
         }
         //
         // The rest of this code is for actually drawing an arc. The HTML standard's description of how to 
         // actually do this is almost entirely math jargon with no explanations, let alone a step-by-step 
         // process that someone who isn't a mathematician could actually follow. So let's break it down a 
         // bit.
         // 
         // We have three points; we can connect them to form an angle. We want to shove a circle (with a 
         // given radius) as deep into that angle as we can. Once we do so, we need to find three pieces 
         // of information in order to proceed: the location of the circle's centerpoint; and the locations 
         // of two "tangent points." When a line is "tangent to" a circle, that means that the line touches 
         // the circle's edge exactly, intersecting that edge at one point; our angle can be thought of as 
         // two lines extending outward from the (by) point, with one passing through (from) and the other 
         // passing through (to), and we need to know the tangent points that each line forms on the circle.
         // 
         // If you try to search the web for information on how to solve this, you're probably going to find 
         // several Mathematics StackExchange answers in which mathematicians try and fail spectacularly to 
         // explain how to do this, because mathematicians tend to be really poor communicators. If you get 
         // very, very lucky, however, you'll stumble upon this answer:
         // 
         // <https://stackoverflow.com/a/51235277>
         // 
         // It's pretty goshdarned challenging to debug, because the author doesn't fully explain how they 
         // derived everything, but they explain just enough to be useful.
         // 
         // The basic idea is that the three points we have form two lines. (The answer above assumes four 
         // points forming two lines; in our case, our lines each share an endpoint.) Given that our circle 
         // is shoved as deep into the angle as possible, and touches the angles, the distance from the 
         // centerpoint to the lines must be the circle's radius. Ergo, we can take our two lines and shift 
         // them inward by the circle's radius, to get two parallel lines that intersect each other exactly 
         // at the circle's centerpoint. Once we find that line intersection, we have the centerpoint.
         // 
         // We can then use several of the intermediate values we computed to also get the two tangent 
         // points. The author makes absolutely no attempt to explain how this works, but they at least 
         // offer some very simple formulae (though the one for `ty2` has a typo in it).
         //
         const QVector2D a = QVector2D(from);
         const QVector2D b = QVector2D(by);
         const QVector2D c = QVector2D(to);
         //
         QVector2D v1; // normalized vector from b to a
         QVector2D v2; // normalized vector from c to b
         QVector2D p1; // vector parallel to v1; intersects (p2) at the circle center
         QVector2D p2; // vector parallel to v2; intersects (p1) at the circle center
         {
            v1 = (b - a).normalized();
            v2 = (c - b).normalized();
            //
            QVector2D bisector = (v2 + -v1).normalized(); // negate (v1) so we can use (b) as our origin
            //
            QVector2D normal1; // vpu1
            normal1.setX(-v1.y());
            normal1.setY( v1.x());
            QVector2D normal2;
            normal2.setX(-v2.y());
            normal2.setY( v2.x());
            //
            bool flip = QVector2D::dotProduct(normal1, bisector) < 0.0; // the normal and the bisector point away from each other if this is true
            if (flip) {
               //
               // In theory you can compute a circle on either side of our lines. We want the circle that 
               // lies on the "inner" side of our lines, so we need to test whether the normal vectors 
               // above point toward the inside (i.e. in the same direction as our bisector) and if not, 
               // we need to flip them.
               //
               normal1 *= -1;
               normal2 *= -1;
            }
            //
            p1 = a + (normal1 * radius);
            p2 = b + (normal2 * radius);
         }
         qreal cross = v1.x() * v2.y() - v2.x() * v1.y(); // The StackOverflow answer calls this variable "den." Dunno why.
         if (abs(cross) < 0.0000001) {
            //
            // Parallel lines or other weird edge-case.
            //
            path->lineTo(by);
            return 0;
         }
         //
         auto pdiff = p2 - p1;
         qreal k1 = (v2.y() * pdiff.x() - v2.x() * pdiff.y()) / cross;
         qreal k2 = (v1.y() * pdiff.x() - v1.x() * pdiff.y()) / cross;
         QVector2D center;
         center.setX(p1.x() + k1 * v1.x());
         center.setY(p1.y() + k1 * v1.y());
         if (false) { // K2 version; should produce the same results as K1 version (maybe only near-identical if floating-point imprecision comes into play); left here as an error check during debugging
            center.setX(p2.x() + k2 * v2.x());
            center.setY(p2.y() + k2 * v2.y());
         }
         //
         QVector2D tangent_1;
         QVector2D tangent_2;
         //
         tangent_1.setX(a.x() + k1 * v1.x());
         tangent_1.setY(a.y() + k1 * v1.y());
         tangent_2.setX(b.x() + k2 * v2.x());
         tangent_2.setY(b.y() + k2 * v2.y());
         //
         // Draw line to first tangent point:
         path->lineTo(tangent_1.toPointF());
         {
            //
            // Qt's "draw arc" function wants a bounding rect for an ellipse, a start angle, and a "sweep" angle. 
            // Computing the bounding rect for a circle with a known centerpoint and radius is trivial. The two 
            // angles are a bit harder, but can be derived by running atan2 on our tangent points (remembering 
            // to first convert them to be relative to the circle centerpoint, as Qt expects).
            // 
            // In tests, we've had to negate them. I've no idea why.
            //
            QRectF rect;
            rect.setTopLeft(center.toPointF() - QPointF(radius, radius));
            rect.setSize({ radius * 2, radius * 2 });
            //
            qreal start;
            qreal sweep;
            {
               //auto local = tangent_1 - b;
               auto local = tangent_1 - center;
               start = cobb::radians_to_degrees(atan2(local.y(), local.x()));
            }
            {
               //auto local = tangent_2 - b;
               auto local = tangent_2 - center;
               sweep = cobb::radians_to_degrees(atan2(local.y(), local.x()));
               sweep -= start;
               if (sweep < 0.0 && start > 0.0)
                  sweep += 360.0;
            }
            //
            path->arcTo(rect, -start, -sweep);
         }
         assert((tangent_2 - QVector2D(path->currentPosition())).length() < 1.0);
         path->lineTo(tangent_2.toPointF());
         path->lineTo(c.toPointF());
         return 0;
      }
      int line_to(lua_State* L) {
         auto*   path = cls::pull_self(L);
         QPointF target;
         auto    result = cobb::lua::pull_qpoint_float(L, 2, target);
         switch (result) {
            case 0:
               break;
            case -1:
               //
               // Not a table.
               //
               cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "table (point) or number (x-coordinate) expected");
               cobb::lua::argcheck(L, lua_isnumber(L, 3), 3, "number (y-coordinate) expected");
               target.setX(lua_tonumber(L, 2));
               target.setY(lua_tonumber(L, 3));
               break;
            case -2:
               cobb::lua::argerror(L, 2, "argument was a table but has no x-coordinate (arg.x or arg[1])");
            case -3:
               cobb::lua::argerror(L, 2, "argument was a table but has no y-coordinate (arg.y or arg[2])");
         }
         path->lineTo(target);
         return 0;
      }
      int move_to(lua_State* L) {
         auto*   path = cls::pull_self(L);
         QPointF target;
         auto    result = cobb::lua::pull_qpoint_float(L, 2, target);
         switch (result) {
            case 0:
               break;
            case -1:
               //
               // Not a table.
               //
               cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "table (point) or number (x-coordinate) expected");
               cobb::lua::argcheck(L, lua_isnumber(L, 3), 3, "number (y-coordinate) expected");
               target.setX(lua_tonumber(L, 2));
               target.setY(lua_tonumber(L, 3));
               break;
            case -2:
               cobb::lua::argerror(L, 2, "argument was a table but has no x-coordinate (arg.x or arg[1])");
            case -3:
               cobb::lua::argerror(L, 2, "argument was a table but has no y-coordinate (arg.y or arg[2])");
         }
         path->moveTo(target);
         return 0;
      }
   }
   namespace _getters {
   }
   namespace _setters {
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         if (lua_gettop(L) > 0)
            cobb::lua::error(L, "the raster_draw_path.new function should not be called with a colon or passed any arguments");
         //
         _wrapped_path* instance = _wrapped_path::create(L); // pushes a userdata onto the Lua stack
         {
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [instance]() {
               instance->path = new QPainterPath;
            };
            send_script_task(*task);
            delete task;
         }
         return 1;
      }
      int is(lua_State* L) {
         auto* wrapper = classes::cast_to_class(L, 1, cls::metatable_key);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "__close", &_wrapped_path::__close }, // This userdata doesn't derive from (wrapper), so it needs its own GC code
      { "__gc",    &_wrapped_path::__gc },    //
      //
      { "add_ellipse", &_methods::add_ellipse },
      { "arc_to",      &_methods::arc_to },
      { "line_to",     &_methods::line_to },
      { "move_to",     &_methods::move_to },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }

   /*static*/ QPainterPath* cls::pull(lua_State* L, int stack_pos) {
      auto* wrapper = (_wrapped_path*) classes::cast_to_class(L, stack_pos, cls::metatable_key);
      if (!wrapper)
         return nullptr;
      return wrapper->path;
   }
   /*static*/ QPainterPath* cls::pull_self(lua_State* L) {
      auto* wrapper = (_wrapped_path*) classes::cast_to_class(L, 1, cls::metatable_key);
      if (!wrapper)
         cobb::lua::error(L, "raster_draw_path method called on bad self");
      return wrapper->path;
   }
}