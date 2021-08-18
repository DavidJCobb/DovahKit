#include "raster.h"
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/for_each_in_array.h"
#include "../../../helpers/lua/istablelike.h"
#include "../../../helpers/lua/qt_point.h"
#include "../../../helpers/lua/qt_variant.h"
#include "../../../helpers/lua/set_top_on_exit.h"
#include "../../../helpers/lua/tostringex.h"
#include "../../../helpers/lua/warning.h"
#include "../../../helpers/rotation.h"
#include "../../constants/qt_graphics.h"
#include "../../core/subsystems/coordinator.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/resources.h"
#include "../../core/subsystems/userdata.h"
#include "../../push_native_object.h"

#include "../../tasks/s2m/lambda.h"
#include "../../tasks/s2m/ui_read_lambda.h"
#include "../../tasks/s2m/ui_write_lambda.h"

#include "../../api_helpers/qt_color.h"

#include "../misc/raster_draw_path.h"

#include <QPainter>
#include <QPainterPath>

namespace {
   using namespace dovahscript;
   using cls = wrappers::resource::raster;

   static constexpr auto default_painter_hints = QPainter::Antialiasing | QPainter::TextAntialiasing;

   namespace _helpers {
      inline QRgb adapt_to_format(QRgb in) noexcept {
         if constexpr (desired_qt_pixel_format == QImage::Format::Format_ARGB32_Premultiplied)
            return qPremultiply(in);
         return in;
      }
      inline QRgb adapt_from_format(QRgb in) noexcept {
         if constexpr (desired_qt_pixel_format == QImage::Format::Format_ARGB32_Premultiplied)
            return qUnpremultiply(in);
         return in;
      }

      void pull_fill_color(lua_State* L, int index, QBrush& brush, bool optional = false) {
         index = lua_absindex(L, index);
         if (optional && lua_isnoneornil(L, index)) {
            brush.setColor(QColorConstants::Transparent);
            return;
         }
         std::string error;
         QColor      color = api_helpers::protected_pull_color(L, index, error);
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
               stop.second = api_helpers::protected_pull_color(L, -1, error);
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
            int size = stops.size();
            for (int i = 1; i < size; ++i) {
               if (stops[i].first == stops[i - 1].first) {
                  cobb::lua::warning(L, "some of the color stops defined in options.fill_gradient.stops have identical positions, and may be discarded");
                  break;
               }
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

      QRgb blend_pixel(QColor src, QRgb dst_rgba) {
         constexpr size_t index_a = std::endian::native == std::endian::little ? 3 : 0;
         constexpr size_t index_r = std::endian::native == std::endian::little ? 2 : 1;
         constexpr size_t index_g = std::endian::native == std::endian::little ? 1 : 2;
         constexpr size_t index_b = std::endian::native == std::endian::little ? 0 : 3;
         //
         auto* dst = (uint8_t*)&dst_rgba;
         //
         qreal src_alpha = (qreal)src.alpha() / 255;
         qreal dst_alpha = (qreal)dst[index_a] / 255;
         qreal inv_src_alpha   = 1.0 - src_alpha;
         qreal final_dst_alpha = dst_alpha * inv_src_alpha;
         //
         QRgb  out;
         auto* out_ptr = (uint8_t*)&out;
         out_ptr[index_a] = src.alpha() + (dst[index_a] * inv_src_alpha);
         out_ptr[index_r] = src.red()   * src_alpha + dst[index_r] * final_dst_alpha;
         out_ptr[index_g] = src.green() * src_alpha + dst[index_g] * final_dst_alpha;
         out_ptr[index_b] = src.blue()  * src_alpha + dst[index_b] * final_dst_alpha;
         //
         return out;
      }
      namespace {
         inline constexpr uint16_t _divide_by_255(uint16_t x) noexcept {
            return ((x + 1) * 257) >> 16;
         }
      }
      QRgb blend_pixel_premul(QRgb color_a, QRgb color_b) { // fast; very slightly inaccurate
         unsigned int alpha = qAlpha(color_a);
         //
         uint32_t a  = alpha + _divide_by_255(qAlpha(color_b) * alpha);
         uint32_t rb = (color_a & 0xFF00FF) + ((alpha * (color_b & 0xFF00FF)) >> 8);
         uint32_t g  = (color_a & 0x00FF00) + ((alpha * (color_b & 0x00FF00)) >> 8);
         return (rb & 0xFF00FF) | (g & 0x00FF00) | (a << 0x18);
      }
   }

   enum class trait {
      required,
      optional,
      skipped,
   };
   struct paint_operation_param_request {
      trait fill_color = trait::optional; // also controls fill_gradient; either must be present
      trait line_color = trait::optional;
      trait line_join  = trait::optional;
      trait line_width = trait::optional;
   };
   void _pull_paint_parameters(lua_State* L, int index, QBrush& brush, QPen& pen, QRectF bound, const paint_operation_param_request request) {
      if (request.fill_color != trait::skipped) {
         lua_getfield(L, index, "fill_color");
         _helpers::pull_fill_color(L, -1, brush, true);
         lua_getfield(L, index, "fill_gradient");
         _helpers::pull_fill_gradient(L, -1, brush, bound);
         lua_pop(L, 2);
         //
         if (request.fill_color != trait::optional) {
            if (!brush.gradient() && !brush.color().isValid())
               cobb::lua::error(L, "neither options.fill_color nor options.fill_gradient appear to have been specified");
         }
      }
      if (request.line_color != trait::skipped) {
         lua_getfield(L, index, "line_color");
         if (lua_isnoneornil(L, -1)) {
            if (request.line_color != trait::optional)
               cobb::lua::error(L, "options.line_color was not specified");
            pen.setColor(Qt::GlobalColor::transparent);
         } else {
            std::string error;
            QColor      color = api_helpers::protected_pull_color(L, -1, error);
            if (!color.isValid()) {
               if (error.empty())
                  cobb::lua::error(L, "options.line_color was not a valid color");
               cobb::lua::error(L, "options.line_color was not a valid color: %s", error.c_str());
            }
            pen.setColor(color);
         }
         lua_pop(L, 1);
      }
      if (request.line_width != trait::skipped) {
         lua_getfield(L, index, "line_width");
         if (lua_isnoneornil(L, -1)) {
            if (request.line_color != trait::optional)
               cobb::lua::error(L, "options.line_width was not specified");
            pen.setWidthF(1.0F);
         } else {
            if (!lua_isnumber(L, -1))
               cobb::lua::error(L, "options.line_width was neither nil nor a number");
            qreal width = lua_tonumber(L, -1);
            if (width <= 0)
               cobb::lua::error(L, "options.line_width was a number less than or equal to zero");
            pen.setWidthF(width);
         }
         lua_pop(L, 1);
      }
      if (request.line_join != trait::skipped) {
         lua_getfield(L, index, "line_join");
         if (lua_isnoneornil(L, -1)) {
            if (request.line_join != trait::optional)
               cobb::lua::error(L, "options.line_join was not specified");
            pen.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
         } else {
            static constexpr std::array join_styles = {
               std::pair{ Qt::PenJoinStyle::BevelJoin, "bevel" },
               std::pair{ Qt::PenJoinStyle::MiterJoin, "miter" },
               std::pair{ Qt::PenJoinStyle::RoundJoin, "round" },
            };
            if (!lua_isstring(L, -1))
               cobb::lua::error(L, "options.line_join was neither nil nor a string");
            auto* str   = lua_tostring(L, -1);
            bool  found = false;
            for (auto& pair : join_styles) {
               if (stricmp(str, pair.second) == 0) {
                  found = true;
                  pen.setJoinStyle(pair.first);
                  break;
               }
            }
            if (!found)
               cobb::lua::error(L, "options.line_join was an unrecognized value: \"%s\"", str);
         }
         lua_pop(L, 1);
      }
   }

   namespace _methods {
      int blend_pixel(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         //
         int isnum;
         int x = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum,  2, "x-coordinate (integer) expected");
         luaL_argcheck(L, x != 0, 2, "x-coordinate cannot be zero");
         luaL_argcheck(L, x >  0, 2, "x-coordinate cannot be negative");
         int y = lua_tointegerx(L, 3, &isnum);
         luaL_argcheck(L, isnum,  3, "y-coordinate (integer) expected");
         luaL_argcheck(L, y != 0, 3, "y-coordinate cannot be zero");
         luaL_argcheck(L, y >  0, 3, "y-coordinate cannot be negative");
         --x;
         --y;
         QColor color = api_helpers::pull_color(L, 4);
         //
         int w = -1;
         int h = -1;
         self.managed_resource->modify_raster_script_side([x, y, color, &w, &h](QImage& image) {
            assert(image.format() == desired_qt_pixel_format);
            w = image.width();
            h = image.height();
            if (x >= w || y >= h)
               return;
            auto* bytes = (QRgb*)image.scanLine(y);
            if constexpr (desired_qt_pixel_format == QImage::Format::Format_ARGB32_Premultiplied) {
               QRgb over = qPremultiply(color.rgba());
               bytes[x] = _helpers::blend_pixel_premul(over, bytes[x]);
               return;
            }
            if (color.alpha() <= 0)
               return;
            if (color.alpha() >= 255) {
               bytes[x] = color.rgba();
               return;
            }
            bytes[x] = _helpers::adapt_from_format(bytes[x]);
            bytes[x] = _helpers::blend_pixel(color, bytes[x]);
            bytes[x] = _helpers::adapt_to_format(bytes[x]);
         });
         luaL_argcheck(L, x < w, 2, "x-coordinate exceeded the raster's width");
         luaL_argcheck(L, y < h, 3, "y-coordinate exceeded the raster's height");
         //
         return 0;
      }
      int draw_ellipse(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, cobb::lua::istablelike(L, 2), 2, "table (options) expected");
         lua_settop(L, 2);
         //
         QPointF center;
         QSizeF  radii;
         qreal   angle = 0.0;
         //
         lua_getfield(L, 2, "center"); // 3
         if (!_helpers::pull_qpoint_f(L, 3, center)) {
            lua_getfield(L, 2, "x"); // 4
            lua_getfield(L, 2, "y"); // 5
            if (!lua_isnumber(L, 4))
               cobb::lua::error(L, "options.center was unspecified or invalid, and options.x was not a number");
            if (!lua_isnumber(L, 5))
               cobb::lua::error(L, "options.center was unspecified or invalid, and options.y was not a number");
            center.setX(lua_tonumber(L, 4));
            center.setY(lua_tonumber(L, 5));
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
            if (!_helpers::pull_qpoint_f(L, 4, working))
               cobb::lua::error(L, "options.radius was unspecified, and options.radii was unspecified or invalid");
            lua_pop(L, 1);
            radii.setWidth(working.x());
            radii.setHeight(working.y());
         }
         lua_pop(L, 1);
         //
         lua_getfield(L, 2, "angle");
         if (!lua_isnoneornil(L, 3)) {
            if (!lua_isnumber(L, 3))
               cobb::lua::error(L, "options.angle was neither nil nor a number");
            angle = lua_tonumber(L, 3); // DO NOT convert to radians; QPainter::rotate takes degrees
         }
         lua_pop(L, 1);
         //
         center += { -1, -1 }; // Lua values should start from (1, 1)
         //
         QPen   pen;
         QBrush brush = QBrush(Qt::SolidPattern);
         QRectF bound;
         {
            bound.setSize(radii * 2);
            bound.translate(center);
            bound.translate({ -radii.width(), -radii.height() });
         }
         //
         assert(lua_gettop(L) == 2);
         _pull_paint_parameters(L, 2, brush, pen, bound, {
            .fill_color = trait::optional,
            .line_color = trait::optional,
            .line_join  = trait::optional,
            .line_width = trait::optional,
         });
         //
         // Begin drawing:
         //
         if (!self.managed_resource)
            return 0;
         self.managed_resource->modify_raster_script_side([center, radii, angle, pen, brush](QImage& image) {
            QPainter painter(&image);
            painter.setRenderHints(default_painter_hints, true);
            painter.setPen(pen);
            painter.setBrush(brush);
            painter.translate(center);
            painter.rotate(angle);
            painter.drawEllipse({ 0, 0 }, radii.width(), radii.height());
         });
         return 0;
      }
      int draw_line(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, cobb::lua::istablelike(L, 2), 2, "table (options) expected");
         lua_settop(L, 2);
         lua_getfield(L, 2, "from");       // 3
         lua_getfield(L, 2, "to");         // 4
         if (!cobb::lua::istablelike(L, 3)) {
            auto type = lua_type(L, 3);
            cobb::lua::error(L, "options.from must be a table; got %s", lua_typename(L, type));
         }
         if (!cobb::lua::istablelike(L, 4)) {
            auto type = lua_type(L, 4);
            cobb::lua::error(L, "options.to must be a table; got %s", lua_typename(L, type));
         }
         std::array<QPointF, 2> endpoints;
         QPen   pen;
         QBrush brush;
         //
         // Get endpoints:
         //
         {
            constexpr std::array names = { "options.from", "options.to" };
            for (int i = 0; i < 2; ++i) {
               endpoints[i] = cobb::lua::pull_qpointf(L, 3 + i, names[i]);
               endpoints[i] += { -1, -1 }; // Lua uses coordinates from (1, 1)
            }
         }
         //
         // Get pen settings:
         //
         _pull_paint_parameters(L, 2, brush, pen, QRectF(), {
            .fill_color = trait::skipped,
            .line_color = trait::required,
            .line_join  = trait::skipped,
            .line_width = trait::optional,
         });
         //
         // Begin drawing:
         //
         if (!self.managed_resource)
            return 0;
         self.managed_resource->modify_raster_script_side([endpoints, pen](QImage& image) {
            QPainter painter(&image);
            painter.setRenderHints(default_painter_hints, true);
            painter.setPen(pen);
            painter.drawLine(endpoints[0], endpoints[1]);
         });
         return 0;
      }
      int draw_path(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, cobb::lua::istablelike(L, 2), 2, "table (options) expected");
         lua_settop(L, 2);
         //
         lua_getfield(L, 2, "path");
         auto* path = wrappers::raster_draw_path::pull(L, 3);
         if (path == nullptr)
            cobb::lua::error(L, "options.path was not a raster_draw_path instance");
         lua_pop(L, 1);
         //
         QPointF offset = { 0, 0 };
         lua_getfield(L, 2, "offset");
         if (cobb::lua::istablelike(L, 3)) {
            auto code = cobb::lua::pull_qpoint_float(L, 3, offset);
            switch (code) {
               case 0:
                  break;
               case -2:
                  cobb::lua::argerror(L, 2, "options.offset was specified but there was no x-coordinate (options.offset.x or options.offset[1])");
               case -3:
                  cobb::lua::argerror(L, 2, "options.offset was specified but there was no y-coordinate (options.offset.y or options.offset[2])");
            }
         }
         lua_pop(L, 1);
         offset -= { 1.0, 1.0 }; // Lua coords to normal
         //
         QPen   pen;
         QBrush brush      = QBrush(Qt::SolidPattern);
         auto   final_path = path->translated(offset);
         auto   bounds     = final_path.boundingRect();
         //
         assert(lua_gettop(L) == 2);
         _pull_paint_parameters(L, 2, brush, pen, bounds, {
            .fill_color = trait::optional,
            .line_color = trait::optional,
            .line_join  = trait::optional,
            .line_width = trait::optional,
         });
         //
         // Begin drawing:
         //
         if (!self.managed_resource)
            return 0;
         self.managed_resource->modify_raster_script_side([&final_path, pen, brush](QImage& image) {
            QPainter painter(&image);
            painter.setRenderHints(default_painter_hints, true);
            painter.setPen(pen);
            painter.setBrush(brush);
            painter.drawPath(final_path);
         });
         return 0;
      }
      int draw_raster(lua_State* L) {
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
      int draw_rect(lua_State* L) {
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
         rect.translate({ -1, -1 }); // Lua values should start from (1, 1)
         //
         QPen   pen;
         QBrush brush = QBrush(Qt::SolidPattern);
         //
         assert(lua_gettop(L) == 2);
         _pull_paint_parameters(L, 2, brush, pen, rect, {
            .fill_color = trait::optional,
            .line_color = trait::optional,
            .line_join  = trait::optional,
            .line_width = trait::optional,
         });
         //
         // Begin drawing:
         //
         if (!self.managed_resource)
            return 0;
         self.managed_resource->modify_raster_script_side([rect, pen, brush](QImage& image) {
            QPainter painter(&image);
            painter.setRenderHints(default_painter_hints, true);
            painter.setPen(pen);
            painter.setBrush(brush);
            painter.drawRect(rect);
         });
         return 0;
      }
      int fill(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         //
         QColor color = api_helpers::pull_color(L, 2);
         //
         self.managed_resource->modify_raster_script_side([color](QImage& image) {
            image.fill(color);
         });
         //
         return 0;
      }
      int flip(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         bool horizontal = false;
         bool vertical   = false;
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string (flip direction) expected");
         auto* str = lua_tostring(L, 2);
         if (stricmp(str, "horizontal") == 0 || stricmp(str, "h") == 0) {
            horizontal = true;
         } else if (stricmp(str, "vertical") == 0 || stricmp(str, "v") == 0) {
            vertical   = true;
         } else if (stricmp(str, "both") == 0) {
            horizontal = true;
            vertical   = true;
         }
         //
         self.managed_resource->modify_raster_script_side([horizontal, vertical](QImage& image) {
            image = image.mirrored(horizontal, vertical);
         });
         //
         return 0;
      }
      int get_pixel(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         //
         int isnum;
         int x = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum,  2, "x-coordinate (integer) expected");
         luaL_argcheck(L, x != 0, 2, "x-coordinate cannot be zero");
         luaL_argcheck(L, x >  0, 2, "x-coordinate cannot be negative");
         int y = lua_tointegerx(L, 3, &isnum);
         luaL_argcheck(L, isnum,  3, "y-coordinate (integer) expected");
         luaL_argcheck(L, y != 0, 3, "y-coordinate cannot be zero");
         luaL_argcheck(L, y >  0, 3, "y-coordinate cannot be negative");
         --x;
         --y;
         //
         auto image = self.managed_resource->get_raster_script_side();
         assert(image.format() == desired_qt_pixel_format);
         luaL_argcheck(L, x < image.width(),  2, "x-coordinate exceeded the raster's width");
         luaL_argcheck(L, y < image.height(), 3, "y-coordinate exceeded the raster's height");
         auto pixel = _helpers::adapt_from_format(image.pixel(x, y));
         api_helpers::push_color(L, pixel);
         return 1;
      }
      int resize(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         //
         QSize size;
         int isnum;
         size.setWidth(lua_tointegerx(L, 2, &isnum));
         luaL_argcheck(L, isnum,             2, "integer (width) expected");
         luaL_argcheck(L, size.width() > 0,  2, "the width cannot be negative or zero");
         size.setHeight(lua_tointegerx(L, 3, &isnum));
         luaL_argcheck(L, isnum,             3, "integer (height) expected");
         luaL_argcheck(L, size.height() > 0, 3, "the height cannot be negative or zero");
         //
         if (!self.managed_resource)
            return 0;
         //
         self.managed_resource->modify_raster_script_side([size](QImage& image) {
            auto prior = image.size();
            if (prior == size)
               return;
            image = image.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            if (image.format() != desired_qt_pixel_format) // QImage::scaled can change the format, and that isn't documented
               image.convertTo(desired_qt_pixel_format);
         });
         //
         return 0;
      }
      int scale(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         //
         QSizeF size;
         //
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number (scale multiplier) expected");
         size.setWidth(lua_tonumber(L, 2));
         luaL_argcheck(L, size.width() > 0.0, 2, "scale multipliers cannot be negative or zero");
         //
         if (lua_isnoneornil(L, 3)) {
            size.setHeight(size.width());
         } else {
            luaL_argcheck(L, lua_isnumber(L, 3),  3, "number (scale multiplier) expected");
            size.setHeight(lua_tonumber(L, 3));
            luaL_argcheck(L, size.height() > 0.0, 3, "scale multipliers cannot be negative or zero");
         }
         //
         if (!self.managed_resource)
            return 0;
         if (size.width() == 1.0 && size.height() == 1.0)
            return 0;
         //
         self.managed_resource->modify_raster_script_side([size](QImage& image) {
            auto prior = image.size();
            prior.setWidth ((qreal)prior.width()  * size.width());
            prior.setHeight((qreal)prior.height() * size.height());
            if (prior.width() < 1)
               prior.setWidth(1);
            if (prior.height() < 1)
               prior.setHeight(1);
            image = image.scaled(prior, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            if (image.format() != desired_qt_pixel_format) // QImage::scaled can change the format, and that isn't documented
               image.convertTo(desired_qt_pixel_format);
         });
         //
         return 0;
      }
      int set_pixel(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         //
         int isnum;
         int x = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum,  2, "x-coordinate (integer) expected");
         luaL_argcheck(L, x != 0, 2, "x-coordinate cannot be zero");
         luaL_argcheck(L, x >  0, 2, "x-coordinate cannot be negative");
         int y = lua_tointegerx(L, 3, &isnum);
         luaL_argcheck(L, isnum,  3, "y-coordinate (integer) expected");
         luaL_argcheck(L, y != 0, 3, "y-coordinate cannot be zero");
         luaL_argcheck(L, y >  0, 3, "y-coordinate cannot be negative");
         --x;
         --y;
         QColor color = api_helpers::pull_color(L, 4);
         //
         int w = -1;
         int h = -1;
         self.managed_resource->modify_raster_script_side([x, y, color, &w, &h](QImage& image) {
            assert(image.format() == desired_qt_pixel_format);
            w = image.width();
            h = image.height();
            if (x >= w || y >= h)
               return;
            auto* bytes = (QRgb*)image.scanLine(y);
            bytes[x] = _helpers::adapt_to_format(color.rgba());
         });
         luaL_argcheck(L, x < w, 2, "x-coordinate exceeded the raster's width");
         luaL_argcheck(L, y < h, 3, "y-coordinate exceeded the raster's height");
         //
         return 0;
      }
   }
   namespace _getters {
      int height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         auto image = self.managed_resource->get_raster_script_side();
         if (image.isNull())
            return 0;
         lua_pushinteger(L, image.height());
         return 1;
      }
      int width(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         auto image = self.managed_resource->get_raster_script_side();
         if (image.isNull())
            return 0;
         lua_pushinteger(L, image.width());
         return 1;
      }
   }
   namespace _setters {
      int height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum,      3, "integer (height) expected");
         luaL_argcheck(L, value != 0, 3, "height cannot be zero");
         luaL_argcheck(L, value >  0, 3, "height cannot be negative");
         if (!self.managed_resource)
            return 0;
         //
         self.managed_resource->modify_raster_script_side([value](QImage& image) {
            if (image.height() == value)
               return;
            image = image.copy(0, 0, image.width(), value);
         });
         //
         return 0;
      }
      int width(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum,      3, "integer (width) expected");
         luaL_argcheck(L, value != 0, 3, "width cannot be zero");
         luaL_argcheck(L, value >  0, 3, "width cannot be negative");
         if (!self.managed_resource)
            return 0;
         //
         self.managed_resource->modify_raster_script_side([value](QImage& image) {
            if (image.width() == value)
               return;
            image = image.copy(0, 0, value, image.height());
         });
         //
         return 0;
      }
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         core::subsystems::permissions::verify_ui_permissions();
         //
         int    width      = -1;
         int    height     = -1;
         QColor background = Qt::GlobalColor::transparent;
         lua_settop(L, 1);
         {
            int isnum;
            int type = lua_type(L, 1);
            if (type != LUA_TTABLE && type != LUA_TUSERDATA)
               cobb::lua::error(L, "expected options table or userdata as argument; got %s", lua_typename(L, type));
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
               background = api_helpers::pull_color(L, 2);
            }
            lua_pop(L, 1);
         }
         //
         DovahscriptResourceHandle resource = nullptr;
         {
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [width, height, background, &resource]() {
               auto base = QImage(width, height, desired_qt_pixel_format);
               if (base.isNull()) // can occur if the image is too large?
                  return;
               base.fill(background);
               resource = core::subsystems::resources::get().create_resource(base);
            };
            core::subsystems::coordinator::get().send_script_task(*task);
            delete task;
            //
            if (!resource)
               cobb::lua::error(L, "unable to create %dx%dpx raster", width, height);
         }
         push_native_object(resource);
      }
      int is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace dovahscript::wrappers::resource {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "blend_pixel",  &_methods::blend_pixel },
      { "draw_ellipse", &_methods::draw_ellipse },
      { "draw_line",    &_methods::draw_line },
      { "draw_path",    &_methods::draw_path },
      { "draw_raster",  &_methods::draw_raster },
      { "draw_rect",    &_methods::draw_rect },
      { "fill",         &_methods::fill },
      { "flip",         &_methods::flip },      // `raster:flip("horizontal")` or `raster:flip("h")` or `raster:flip("vertical")` or `raster:flip("v")` or `raster:flip("both")`
      { "get_pixel",    &_methods::get_pixel },
      { "resize",       &_methods::resize },
      { "scale",        &_methods::scale },     // `raster:scale(2.0)` or `raster:scale(2.0, 0.5)` given either one size multiplier, or two (width and height respectively)
      { "set_pixel",    &_methods::set_pixel },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "height", &_getters::height },
      { "width",  &_getters::width },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "height", &_setters::height },
      { "width",  &_setters::width },
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}