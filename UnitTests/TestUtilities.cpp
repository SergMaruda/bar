#include "TestUtilities.h"

#include <MatUnitTesting/MatUnitTesting.h>

#include <MatStl\AU_ImportExport.h>
#include <MatStl\MatSaxProject\MatSaxProjectReader.h>
#include <MatStl\MatSaxProject\MatSaxProjectWriter.h>
#include <MatStl\MatSaxProject\MSaxProjectObjectStl.h>
#include <MatStl\MStlTexture.h>
#include <MatStl\MStlTextureParametersLabel.h>
#include <MatStl\MStlTexturefactory.h>
#include <MatStl\Graphs\MStlGraph.h>
#include <MatStl\Graphs\MStlGraphFeature.h>
#include <MatStlSlicer\ConceptLaserParameters.h>

SampleParts g_parts;

SampleParts::SampleParts(void)
: loaded(false)
  {
  m_folder_in  = _T("F:\\_Data\\__STL+\\StrykerSampleLarge\\");
  m_folder_out = _T("F:\\_Data\\__STL+\\StrykerSampleLarge\\Out\\");
  }

void SampleParts::Load(void)
  {
  if (loaded)
    return;

  if (true)
    {
    MatSaxProjectReader reader;
    reader.Read(m_folder_in + _T("Total.mdck"));

    std::vector<hSaxProjectObjectStl> stls;
    reader.GetObjects<MSaxProjectObjectStl>(stls);
    if (stls.size())
      {
      stl_total = *stls[0]->GetStl();
      }
    }
  if (true)
    {
    MatSaxProjectReader reader;
    reader.Read(m_folder_in + _T("Solid.mdck"));

    std::vector<hSaxProjectObjectStl> stls;
    reader.GetObjects<MSaxProjectObjectStl>(stls);
    if (stls.size())
      {
      stl_solid = *stls[0]->GetStl();
      }
    }
  if (true)
    {
    MatSaxProjectReader reader;
    reader.Read(m_folder_in + _T("Base.mdck"));

    std::vector<hSaxProjectObjectStl> stls;
    reader.GetObjects<MSaxProjectObjectStl>(stls);
    if (stls.size())
      {
      stl_base = *stls[0]->GetStl();
      }
    }
  if (true)
    {
    MatSaxProjectReader reader;
    reader.Read(m_folder_in + _T("Porous.mdck"));

    std::vector<hSaxProjectObjectStl> stls;
    reader.GetObjects<MSaxProjectObjectStl>(stls);
    if (stls.size())
      {
      stl_porous = *stls[0]->GetStl();
      }
    }

  AU::ImportPart(stl_cell_solid, _T("F:\\_Data\\__STL+\\vol_texture2.stl"));
  AU::ImportPart(stl_cell_solid_small, _T("F:\\_Data\\__STL+\\vol_texture3.stl"));
  AU::ImportPart(stl_cell_solid_big, _T("F:\\_Data\\__STL+\\vol_texture4.stl"));

  if (true)
    {
    MatSaxProjectReader reader;
    reader.Read(_T("F:\\_Data\\__STL+\\CellSurface.mdck"));

    std::vector<hSaxProjectObjectStl> stls;
    reader.GetObjects<MSaxProjectObjectStl>(stls);
    for(size_t i=0;i<stls.size();i++)
      {
      stl_cell_surface = *stls[i]->GetStl();
      }
    stl_cell_surface.Rescale(2.0, 2.0, 2.0, PNT_3D_LONG());
    }
  /*
  if (true)
    {
    MatSaxProjectReader reader;
    reader.Read(_T("F:\\_Data\\__STL+\\AssemblyComp1.mdck"));

    std::vector<MSaxProjectObjectStl*> stls;
    reader.GetObjects<MSaxProjectObjectStl>(stls);
    for(size_t i=0;i<stls.size();i++)
      {
      stl_comp1 = *stls[i]->GetStl();
      }

    MStlTexture* p_texture = stl_comp1.GetTextures()[0];

    if (p_texture)
      {
      p_texture->GetParametersLabel(true)->m_enabled        = true;
      p_texture->GetParametersLabel(true)->m_font_name      = _T("Arial");
      p_texture->GetParametersLabel(true)->m_text_template  = _T("Test");
      p_texture->GetParametersLabel(true)->m_text_height    = long(5*StdBase::MInternalUnits::Get());
      }
    }*/

  if (true)
    {
    MStlGraph* p_graph = stl_cell_graph_normal.GetGraphs()->AddNewGraph();

    MStlPoint* p_point[2];
    long       size = 40000;
    p_point[0] = stl_cell_graph_normal.AddPoint(0   , 0   , 0);
    p_point[1] = stl_cell_graph_normal.AddPoint(size, size, size);
    p_graph->AddEdge(p_point[0], p_point[1]);
    p_point[0] = stl_cell_graph_normal.AddPoint(size, 0   , 0);
    p_point[1] = stl_cell_graph_normal.AddPoint(0   , size, size);
    p_graph->AddEdge(p_point[0], p_point[1]);
    p_point[0] = stl_cell_graph_normal.AddPoint(size, size, 0);
    p_point[1] = stl_cell_graph_normal.AddPoint(0   , 0   , size);
    p_graph->AddEdge(p_point[0], p_point[1]);
    p_point[0] = stl_cell_graph_normal.AddPoint(0   , size, 0);
    p_point[1] = stl_cell_graph_normal.AddPoint(size, 0   , size);
    p_graph->AddEdge(p_point[0], p_point[1]);
    }

  if (true)
    {
    MStlGraph* p_graph = stl_cell_graph_small.GetGraphs()->AddNewGraph();

    MStlPoint* p_point[2];
    long       size = 20000;
    p_point[0] = stl_cell_graph_small.AddPoint(0   , 0   , 0);
    p_point[1] = stl_cell_graph_small.AddPoint(size, size, size);
    p_graph->AddEdge(p_point[0], p_point[1]);
    p_point[0] = stl_cell_graph_small.AddPoint(size, 0   , 0);
    p_point[1] = stl_cell_graph_small.AddPoint(0   , size, size);
    p_graph->AddEdge(p_point[0], p_point[1]);
    p_point[0] = stl_cell_graph_small.AddPoint(size, size, 0);
    p_point[1] = stl_cell_graph_small.AddPoint(0   , 0   , size);
    p_graph->AddEdge(p_point[0], p_point[1]);
    p_point[0] = stl_cell_graph_small.AddPoint(0   , size, 0);
    p_point[1] = stl_cell_graph_small.AddPoint(size, 0   , size);
    p_graph->AddEdge(p_point[0], p_point[1]);
    }

  if (true)
    {
    MStlGraph* p_graph = stl_cell_graph_big.GetGraphs()->AddNewGraph();

    MStlPoint* p_point[2];
    long       size = 80000;
    p_point[0] = stl_cell_graph_big.AddPoint(0   , 0   , 0);
    p_point[1] = stl_cell_graph_big.AddPoint(size, size, size);
    p_graph->AddEdge(p_point[0], p_point[1]);
    p_point[0] = stl_cell_graph_big.AddPoint(size, 0   , 0);
    p_point[1] = stl_cell_graph_big.AddPoint(0   , size, size);
    p_graph->AddEdge(p_point[0], p_point[1]);
    p_point[0] = stl_cell_graph_big.AddPoint(size, size, 0);
    p_point[1] = stl_cell_graph_big.AddPoint(0   , 0   , size);
    p_graph->AddEdge(p_point[0], p_point[1]);
    p_point[0] = stl_cell_graph_big.AddPoint(0   , size, 0);
    p_point[1] = stl_cell_graph_big.AddPoint(size, 0   , size);
    p_graph->AddEdge(p_point[0], p_point[1]);
    }

  if (true)
    {
    MatSaxProjectReader reader;
    reader.Read(_T("F:\\_Data\\enterprise.mdck"));

    std::vector<hSaxProjectObjectStl> stls;
    reader.GetObjects<MSaxProjectObjectStl>(stls);
    for(size_t i=0;i<stls.size();i++)
      {
      stl_enterprise = *stls[i]->GetStl();
      }
    }
  if (true)
    {
    MatSaxProjectReader reader;
    reader.Read(_T("F:\\_Data\\dragster.mdck"));

    std::vector<hSaxProjectObjectStl> stls;
    reader.GetObjects<MSaxProjectObjectStl>(stls);
    for(size_t i=0;i<stls.size();i++)
      {
      stl_dragster = *stls[i]->GetStl();
      }
    }

  loaded = true;
  }

void FillConceptLaserParameters(MCLParameters& o_parameters)
  {
  MCLProcessParameters* p_set;
  p_set = o_parameters.GetProcessParameters(_T("Base"), true);
  p_set->m_beam_comp = 1000;
  p_set->m_build_parameters.m_laser_focus = 0.1;
  p_set->m_build_parameters.m_laser_power = 400;
  p_set = o_parameters.GetProcessParameters(_T("Solid"), true);
  p_set->m_beam_comp = 1500;
  p_set->m_build_parameters.m_laser_focus = 0.25;
  p_set->m_build_parameters.m_laser_power = 600;
  p_set = o_parameters.GetProcessParameters(_T("Porous"), true);
  p_set->m_beam_comp = 500;
  p_set->m_build_parameters.m_laser_focus = 0.26;
  p_set->m_build_parameters.m_laser_power = 400;
  p_set = o_parameters.GetProcessParameters(_T("Label"), true);
  p_set->m_beam_comp = 2500;
  p_set->m_build_parameters.m_laser_focus = 0.32;
  p_set->m_build_parameters.m_laser_power = 300;
  }

TEST(TestXML)
  {
  g_parts.Load();
  if (true)
    {
    MatSaxProjectWriter writer;

    writer.AddStl(&g_parts.stl_enterprise);
    writer.AddStl(&g_parts.stl_dragster);
    writer.Write(_T("F:\\_Data\\Stls.xml"));
    }
  }