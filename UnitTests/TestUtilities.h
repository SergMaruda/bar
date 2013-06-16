#pragma once

#include <MatStl\stl_base.h>

///////////////////////////////////////////////////////////////////////////////

struct SampleParts
  {
  std::wstring  m_folder_in;
  std::wstring  m_folder_out;

  bool        loaded;
  MStlBase    stl_total;
  MStlBase    stl_base;
  MStlBase    stl_solid;
  MStlBase    stl_porous;
  MStlBase    stl_cell_solid;
  MStlBase    stl_cell_solid_small;
  MStlBase    stl_cell_solid_big;
  MStlBase    stl_cell_surface;
  MStlBase    stl_cell_graph_small;
  MStlBase    stl_cell_graph_normal;
  MStlBase    stl_cell_graph_big;
  MStlBase    stl_cell_hybrid;

  MStlBase    stl_enterprise;
  MStlBase    stl_dragster;

  SampleParts(void);
  void Load(void);
  };

///////////////////////////////////////////////////////////////////////////////

extern SampleParts g_parts;

class MCLParameters;
void FillConceptLaserParameters(MCLParameters& o_parameters);