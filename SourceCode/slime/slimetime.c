/* This file is an image processing operation for GEGL
 *
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.
 *
 * Credit to Øyvind Kolås (pippin) for major GEGL contributions
 * 2025, beaver, Slime

cubism tile-size=4.5 seed=33 gaussian-blur  abyss-policy=none  clip-extent=false std-dev-x=1 std-dev-y=1  hue-chroma lightness=7 

id=1 
src aux=[ ref=1 ripple amplitude=32 abyss-policy=none angle=6 
median-blur  abyss-policy=none neighborhood=diamond radius=65 alpha-percentile=100 cubism tile-size=14 seed=33 median-blur  abyss-policy=none radius=20 median-blur  abyss-policy=none radius=20 color-overlay value=#ff0000 ] 

id=2 over aux=[ ref=2 inner-glow value=#000000 cover=66 radius=3 opacity=0.9 ]
id=3 over aux=[ ref=3  lb:glassovertext azimuth=23 ]
id=4 over aux=[ ref=4  lb:glassovertext azimuth=203 ]


end of syntax
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES


property_boolean (policy, _("Isolate"), FALSE)
  description    (_("Should the slime be behind the image or isolated. If true it will isolate the slime"))

property_int (size, _("Size"), 45)
    description (_("Scaling of the slime"))
    value_range (15, 80)
    ui_meta ("unit", "degree")


property_double (shade, _("Shade around the slime"), 3.0)
    description (_("Shading the edges of the slime"))
    value_range (1.0, 10.0)
    ui_meta ("unit", "degree")

property_double (azimuth1, _("Half azimuth 1"), 23.0)
    description (_("Half azimuth part one of two"))
    value_range (0.0, 180.0)
    ui_meta ("unit", "degree")

property_double (azimuth2, _("Half azimuth 2"), 203.0)
    description (_("Half azimuth part two of two"))
    value_range (180.0, 360.0)
    ui_meta ("unit", "degree")

property_double (ripple_amplitude, _("Sine Amplitude"), 32.0)
    description (_("Internal ripple's amplitude"))
    value_range (20.0, 60.0)
    ui_meta ("unit", "degree")

property_double (ripple_angle, _("Sine angle"), 6.0)
    description (_("Internal ripple's angle"))
    value_range (0.0, 360.0)
    ui_meta ("unit", "degree")

property_double (gloss_opacity, _("White Gloss Opacity"), 1.2)
    description (_("The opacity of the white gloss"))
    value_range (0.5, 1.2)
    ui_meta ("unit", "degree")

property_seed (seed, _("Seed"), rand)
    description (_("Seed of the slime"))

property_color (color, _("Color"), "#00c91f" )
    description (_("Color of the slime"))

#else

#define GEGL_OP_META
#define GEGL_OP_NAME     lb_slime
#define GEGL_OP_C_SOURCE slimetime.c

#include "gegl-op.h"

/*starred nodes go inside typedef struct */

typedef struct
{
 GeglNode *input;
 GeglNode *medianscale;
 GeglNode *median1;
 GeglNode *median2;
 GeglNode *cubism1;
 GeglNode *cubism2;
 GeglNode *blur;
 GeglNode *light;
 GeglNode *normal1;
 GeglNode *normal2;
 GeglNode *normal3;
 GeglNode *replace;
 GeglNode *idref1;
 GeglNode *idref2;
 GeglNode *idref3;
 GeglNode *idref4;
 GeglNode *behind;
 GeglNode *ripple;
 GeglNode *glassoverlay1;
 GeglNode *glassoverlay2;
 GeglNode *innerglow;
 GeglNode *color;
 GeglNode *output;
}State;

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglProperties *o = GEGL_PROPERTIES (operation);
  GeglColor *colorsave = gegl_color_new ("#00c91f");
  GeglColor *colorsavetwo = gegl_color_new ("#000000");
  GeglColor *colorsavethree = gegl_color_new ("#ffffff");
  GeglColor *colorsavefour = gegl_color_new ("#ffffff");

  State *state = o->user_data = g_malloc0 (sizeof (State));

/*new child node list is here, this is where starred nodes get defined

 state->newchildname = gegl_node_new_child (gegl, "operation", "lb:name", NULL);*/
  state->input    = gegl_node_get_input_proxy (gegl, "input");
  state->output   = gegl_node_get_output_proxy (gegl, "output");

 state->color = gegl_node_new_child (gegl, "operation", "gegl:color-overlay", "value", colorsave, NULL);
 state->replace = gegl_node_new_child (gegl, "operation", "gegl:src", NULL);
 state->light = gegl_node_new_child (gegl, "operation", "gegl:hue-chroma", "lightness", 7.0, NULL);
 state->behind = gegl_node_new_child (gegl, "operation", "gegl:dst-over", NULL);
 state->blur = gegl_node_new_child (gegl, "operation", "gegl:gaussian-blur", "abyss-policy", 0, NULL);
 state->normal1 = gegl_node_new_child (gegl, "operation", "gegl:over", NULL);
 state->normal2 = gegl_node_new_child (gegl, "operation", "gegl:over", NULL);
 state->normal3 = gegl_node_new_child (gegl, "operation", "gegl:over", NULL);
 state->glassoverlay1 = gegl_node_new_child (gegl, "operation", "lb:glassovertext", "azimuth", 23.0, "hyperopacity", 1.2, "color", colorsavethree,  NULL);
 state->glassoverlay2 = gegl_node_new_child (gegl, "operation", "lb:glassovertext", "azimuth", 203.0, "hyperopacity", 1.2, "color", colorsavefour,  NULL);
 state->innerglow = gegl_node_new_child (gegl, "operation", "gegl:inner-glow", "cover", 66.0, "opacity", 1.0, "value", colorsavetwo, NULL);
 state->cubism1 = gegl_node_new_child (gegl, "operation", "gegl:cubism", "tile-size", 4.5,  NULL);
 state->cubism2 = gegl_node_new_child (gegl, "operation", "gegl:cubism", "tile-size", 14.0, NULL);
 state->ripple = gegl_node_new_child (gegl, "operation", "gegl:ripple", "amplitude", 112.0, "angle", 14.0, "abyss-policy", 0, NULL);
 state->medianscale = gegl_node_new_child (gegl, "operation", "gegl:median-blur", "alpha-percentile", 100.0, "neighborhood", 2, "radius", 65, "abyss-policy", 0,   NULL);
 state->median1 = gegl_node_new_child (gegl, "operation", "gegl:median-blur", "abyss-policy", 0, "radius", 20,   NULL);
 state->median2 = gegl_node_new_child (gegl, "operation", "gegl:median-blur", "abyss-policy", 0, "radius", 20,  NULL);
 state->idref1 = gegl_node_new_child (gegl, "operation", "gegl:nop", NULL);
 state->idref2 = gegl_node_new_child (gegl, "operation", "gegl:nop", NULL);
 state->idref3 = gegl_node_new_child (gegl, "operation", "gegl:nop", NULL);
 state->idref4 = gegl_node_new_child (gegl, "operation", "gegl:nop", NULL);
/*meta redirect property to new child orders go here

 gegl_operation_meta_redirect (operation, "propertyname", state->newchildname,  "originalpropertyname");

*/

}

static void update_graph (GeglOperation *operation)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  State *state = o->user_data;
  if (!state) return;

  if (!o->policy)
  gegl_node_link_many (state->input, state->behind, state->output,  NULL);
  gegl_node_connect (state->behind, "aux", state->normal3, "output");
  gegl_node_link_many (state->input,  state->cubism1, state->blur, state->light, state->idref1, state->replace, state->idref2, state->normal1, state->idref3, state->normal2, state->idref4, state->normal3,   NULL);
  gegl_node_connect (state->replace, "aux", state->color, "output");
  gegl_node_link_many (state->idref1, state->ripple, state->medianscale, state->cubism2, state->median1, state->color,  NULL);
  gegl_node_connect (state->normal1, "aux", state->innerglow, "output");
  gegl_node_connect (state->normal2, "aux", state->glassoverlay1, "output");
  gegl_node_link_many (state->idref2, state->innerglow,  NULL);
  gegl_node_link_many (state->idref3, state->glassoverlay1,  NULL);
  gegl_node_connect (state->normal3, "aux", state->glassoverlay2, "output");
  gegl_node_link_many (state->idref4, state->glassoverlay2,  NULL);


  if  (o->policy)
  gegl_node_link_many (state->input, state->cubism1, state->blur, state->light, state->idref1, state->replace, state->idref2, state->normal1, state->idref3, state->normal2, state->idref4, state->normal3, state->output,  NULL);
  gegl_node_connect (state->replace, "aux", state->color, "output");
  gegl_node_link_many (state->idref1, state->ripple, state->medianscale, state->cubism2, state->median1, state->median2, state->color,  NULL);
  gegl_node_connect (state->normal1, "aux", state->innerglow, "output");
  gegl_node_connect (state->normal2, "aux", state->glassoverlay1, "output");
  gegl_node_link_many (state->idref2, state->innerglow,  NULL);
  gegl_node_link_many (state->idref3, state->glassoverlay1,  NULL);
  gegl_node_connect (state->normal3, "aux", state->glassoverlay2, "output");
  gegl_node_link_many (state->idref4, state->glassoverlay2,  NULL);

/*optional connect from and too is here
  gegl_node_connect (state->blendmode, "aux", state->lastnodeinlist, "output"); */

 gegl_operation_meta_redirect (operation, "size", state->medianscale,  "radius");
 gegl_operation_meta_redirect (operation, "color", state->color,  "value");
 gegl_operation_meta_redirect (operation, "seed", state->cubism1,  "seed");
 gegl_operation_meta_redirect (operation, "seed", state->cubism2,  "seed");
 gegl_operation_meta_redirect (operation, "ripple_amplitude", state->ripple,  "amplitude");
 gegl_operation_meta_redirect (operation, "ripple_angle", state->ripple,  "angle");
 gegl_operation_meta_redirect (operation, "azimuth1", state->glassoverlay1,  "azimuth");
 gegl_operation_meta_redirect (operation, "azimuth2", state->glassoverlay2,  "azimuth");
 gegl_operation_meta_redirect (operation, "shade", state->innerglow,  "radius");
 gegl_operation_meta_redirect (operation, "gloss_opacity", state->glassoverlay1,  "hyperopacity");
 gegl_operation_meta_redirect (operation, "gloss_opacity", state->glassoverlay2,  "hyperopacity");
}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;
GeglOperationMetaClass *operation_meta_class = GEGL_OPERATION_META_CLASS (klass);
  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;
  operation_meta_class->update = update_graph;

  gegl_operation_class_set_keys (operation_class,
    "name",        "lb:slime",
    "title",       _("Slime"),
    "reference-hash", "iquitwatchingtotaldramaislandcuzscarygirlgotkicked",
    "description", _("Slimey gooey substance"),
/*<Image>/Colors <Image>/Filters are top level menus in GIMP*/
    "gimp:menu-path", "<Image>/Filters/Text Styling",
    "gimp:menu-label", _("Slime..."),
    NULL);
}

#endif
