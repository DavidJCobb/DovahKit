#pragma once
#include <array>
#include "./hardcoded.h"

namespace dovahkit::subsystems::worldedit {
   extern const std::array<gizmo_color_scheme, 10> hardcoded_gizmo_color_schemes = std::array{
      gizmo_color_scheme{
         .name = "Standard",
         .axes = {
            gizmo_color_scheme::rgb{ 255, 0, 0 },
            gizmo_color_scheme::rgb{ 0, 255, 0 },
            gizmo_color_scheme::rgb{ 0, 0, 255 }
         },
         .highlight = { 255, 255, 0 }
      },
      gizmo_color_scheme{
         .name = "Cotton Candy",
         .axes = {
            gizmo_color_scheme::rgb{ 245, 169, 184 },
            gizmo_color_scheme::rgb{ 240, 240, 240 },
            gizmo_color_scheme::rgb{  91, 206, 250 }
         },
         .highlight = { 255, 255, 0 }
      },
      gizmo_color_scheme{
         .name = "Glacier",
         .axes = {
            gizmo_color_scheme::rgb{ 240, 240, 240 },
            gizmo_color_scheme::rgb{  38, 206, 170 },
            gizmo_color_scheme::rgb{  80,  73, 203 }
         },
         .highlight = { 255, 255, 0 }
      },
      gizmo_color_scheme{
         .name = "Halftone",
         .axes = {
            gizmo_color_scheme::rgb{ 255,  33, 140 },
            gizmo_color_scheme::rgb{ 255, 216,   0 },
            gizmo_color_scheme::rgb{  33, 177, 255 }
         },
         .highlight = { 240, 240, 240 }
      },
      gizmo_color_scheme{
         .name = "Late Day Sea",
         .axes = {
            gizmo_color_scheme::rgb{ 214,   2, 112 },
            gizmo_color_scheme::rgb{ 155,  79, 150 },
            gizmo_color_scheme::rgb{   0,  56, 168 }
         },
         .highlight = { 255, 255, 0 }
      },
      gizmo_color_scheme{
         .name = "Mint Steel",
         .axes = {
            gizmo_color_scheme::rgb{ 169, 169, 169 },
            gizmo_color_scheme::rgb{  61, 165,  66 },
            gizmo_color_scheme::rgb{  12,  12,  12 }
         },
         .highlight = { 255, 255, 0 }
      },
      gizmo_color_scheme{
         .name = "Royal Bee",
         .axes = {
            gizmo_color_scheme::rgb{ 155,  89, 208 },
            gizmo_color_scheme::rgb{ 255, 244,  51 },
            gizmo_color_scheme::rgb{  45,  45,  45 }
         },
         .highlight = { 240, 240, 240 }
      },
      gizmo_color_scheme{
         .name = "Royal Steel",
         .axes = {
            gizmo_color_scheme::rgb{ 128,   0, 128 },
            gizmo_color_scheme::rgb{ 163, 163, 163 },
            gizmo_color_scheme::rgb{  12,  12,  12 }
         },
         .highlight = { 255, 255, 0 }
      },
      gizmo_color_scheme{
         .name = "Sea at Sunset",
         .axes = {
            gizmo_color_scheme::rgb{ 213,  45,   0 },
            gizmo_color_scheme::rgb{ 240, 240, 240 },
            gizmo_color_scheme::rgb{ 163,   2,  98 }
         },
         .highlight = { 64, 192, 240 }
      },
      gizmo_color_scheme{
         .name = "Shoreline",
         .axes = {
            gizmo_color_scheme::rgb{ 236, 205,   0 },
            gizmo_color_scheme::rgb{ 240, 240, 240 },
            gizmo_color_scheme::rgb{  68, 174, 220 }
         },
         .highlight = { 255, 255, 0 }
      },
   };
}