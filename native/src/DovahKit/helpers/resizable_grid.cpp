#include "resizable_grid.h"

namespace cobb {
   constexpr bool test = []() -> bool {
      resizable_square_grid<int> gr(5);

      gr.at(0, 0) = 5;
      gr.resize(3);

      return gr.at(0, 0) == 5;

      return gr.area() == 25;


      return false;
   }();
   constexpr auto test2 = []() {
      resizable_square_grid<int> gr(5);
      return gr.right();
   }();
   constexpr auto test3 = []() {
      resizable_square_grid<int> gr(4);
      return gr.right();
   }();

   
   constexpr bool test4 = []() -> bool {
      resizable_grid<int> gr(5, 5);

      gr.at(0, 0) = 5;
      gr.resize(3, 3);

      return gr.at(0, 0) == 5;

      return gr.area() == 25;


      return false;
   }();
}