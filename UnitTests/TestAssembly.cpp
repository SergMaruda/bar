#include "TestUtilities.h"

#include "..\Assemblies\Assembly.h"
#include "..\Assemblies\AssemblyComponent.h"
#include "..\Assemblies\ShapeDetailed.h"
#include "..\Assemblies\AssemblyComponentTypeInfo.h"
#include "..\Assemblies\SurfaceTextures\SurfaceTextures.h"
#include "..\Assemblies\Labels\Labels.h"
#include "..\Assemblies\Labels\LabelParameters.h"
#include "..\Operations\OperationStlExport.h"
#include "..\Slicer\SliceableAssembly.h"
#include "..\Slicer\SliceablePlatform.h"
#include "..\StlConvert\StlConvertAssembly.h"
#include "..\StlConvert\StlConvertPlatform.h"
#include "..\SliceStack\SaxProjectObjectSliceStack.h"
#include "..\Operations\OperationSliceContainerToLayeredStl.h"
#include "..\Platforms\Platform.h"
#include "..\Platforms\PartInstance.h"

#include <MatUnitTesting/MatUnitTesting.h>
#include <MatBase\MFile.h>
#include <MatBase\TimeCount.h>

#include <MatSAX\ISaxPersist.h>
#include <MatSAX\MSaxWriter.h>
#include <MatSAX\MSaxTagWriter.h>
#include <MatSAX\MSaxReader.h>

#include <MatStl\stl_base.h>
#include <MatStl\stl_file_io.h>
#include <MatStl\AU_ImportExport.h>
#include <MatStl\MatSaxProject\MatSaxProjectReader.h>
#include <MatStl\MatSaxProject\MatSaxProjectWriter.h>
#include <MatStl\MatSaxProject\MSaxProjectObjectStl.h>
#include <MatStl\stl_mask_factory.h>
#include <MatStl\stl_dependency.h>
#include <MatStl\MStlDependencyIOSample.h>
#include <MatStl\MStlTexture.h>
#include <MatStl\MStlTextureParametersLabel.h>
#include <MatStl\MStlTexturefactory.h>

#include <MatStlSlicer\MOperatorCreateSliceStack.h>
#include <MatStlSlicer\MOperatorWriteSliceFile.h>
#include <Mat2D\slice_formats.h>
#include <Mat2DSlices\M2DSliceStack.h>
#include <Mat2DSlices\Properties\PersistentProperties.h>
#include <Mat2DSlices\Properties\PersistentProperty.h>
#include <Mat2DSlices\Properties\PropertyNames.h>
#include <Mat2DSlices\Properties\IndexedPropertySets.h>
#include <MatStdBase\InternalUnits.h>

#include <MatMachines\MSAXMachine.h>

///////////////////////////////////////////////////////////////////////////////

TEST(TestStlAssemblies1)
  {
  return;
  MStlBase stl;

  MatAM::AssemblyStlDependency::CreateInstance(&stl);

  AU::ImportPart(stl, _T("F:\\_Data\\__FileFormat\\rook_0.stl"));

  if (true)
    {
    MatAM::Assembly* p_assembly = MatAM::AssemblyStlDependency::GetInstance(&stl);
    MatAM::AssemblyComponent* p_component_subvolume = p_assembly->AddComponent();
    }

  if (true)
    {
    MatSaxProjectWriter writer;

    MMTransformMatrix matrix;

    matrix.SetElement(0, 0, 0 );
    matrix.SetElement(0, 1, -1);
    matrix.SetElement(1, 0, 1 );
    matrix.SetElement(1, 1, 0 );
    matrix.SetElement(2, 2, 1);
    matrix.SetElement(0, 3, 1);
    matrix.SetElement(1, 3, 2);
    matrix.SetElement(2, 3, 3);

    writer.AddStl(&stl, matrix);
    writer.Write(_T("F:\\_Data\\__FileFormat\\rook_0_MSP.mdck"));
    }

  if (true)
    {
    MatSaxProjectReader reader;
    reader.Read(_T("F:\\_Data\\__FileFormat\\rook_0_MDCK.mdck"));

    MatSaxProjectReader::Stls::iterator it, end_it;

    it     = reader.GetStls().begin();
    end_it = reader.GetStls().end();
    while (it != end_it)
      {
      MMTransformMatrix matrix = reader.GetTransformation(*it);

      delete *it;
      ++it;
      }
    }
  }

///////////////////////////////////////////////////////////////////////////////

TEST(TestStlAssemblies2)
  {
  g_parts.Load();

  MStlBase my_stl(g_parts.stl_total);
  MatAM::AssemblyStlDependency::CreateInstance(&my_stl);

  MatAM::AssemblyComponent* p_component_base   = NULL;
  MatAM::AssemblyComponent* p_component_solid  = NULL;
  MatAM::AssemblyComponent* p_component_porous = NULL;

  if (true)
    {
    MatAM::Assembly* p_assembly = MatAM::AssemblyStlDependency::GetInstance(&my_stl);

    // main component is only for help
    p_assembly->GetMainComponent().GetBuildMode().SetMode(MatAM::AssemblyComponentBuildMode::eVoid);
    p_assembly->GetMainComponent().SetVisible(false);
    p_assembly->GetMainComponent().SetNameProcessParameters(_T("Default"));

    if (true)
      {
      p_component_base = p_assembly->AddComponent();
      p_component_base->GetShape().SetStl(g_parts.stl_base);
      p_component_base->GetTypeInfo().SetType(MatAM::eSubComponentGeneric);
      p_component_base->SetNameProcessParameters(_T("Base"));
      //MatAM::Label& label = p_component_subvolume1->GetLabels().AddLabel(p_component_subvolume1->GetShape().GetStl()->GetTextures()[0]);
      //label.GetParameters().m_mode_3d = MatAM::LabelParameters::eSolid;
      //label.GetParameters().m_z[0] = long(-1*StdBase::MInternalUnits::Get());
      //label.GetParameters().m_z[1] = long(1*StdBase::MInternalUnits::Get());
      MatAM::VolumeStructure& structure = p_component_base->GetVolumeStructures().AddVolumeStructure();
      structure.SetEnabled(true);
      structure.SetUnitShape(g_parts.stl_cell_graph_big);
      structure.GetUnitShape().SetIsSolid(false);
      structure.SetGraphRandomization(0, 0.3);
      structure.SetGraphRandomization(1, 0.3);
      structure.SetGraphRandomization(2, 0.3);
      //structure.GetUnitShape().SetThickness2D(long(0.2*StdBase::MInternalUnits::Get()));
      }
    if (true)
      {
      p_component_solid = p_assembly->AddComponent();
      p_component_solid->GetShape().SetStl(g_parts.stl_solid);
      p_component_solid->GetTypeInfo().SetType(MatAM::eSubComponentGeneric);
      p_component_solid->SetNameProcessParameters(_T("Solid"));

      MStlTexture* p_texture = p_component_solid->GetShape().GetStl()->GetTextureFactory()->GetTextureWithName(_T("Label1"));
      if (p_texture)
        {
        MatAM::Label& label = p_component_solid->GetLabels().AddLabel(p_texture);
        label.GetParameters().m_mode_3d       = MatAM::LabelParameters::eRaised;
        label.GetParameters().m_z[0]          = long(-0.50*StdBase::MInternalUnits::Get());
        label.GetParameters().m_z[1]          = long(+0.50*StdBase::MInternalUnits::Get());
        label.GetParameters().m_text_height   = long(3*StdBase::MInternalUnits::Get());
        label.GetParameters().m_text_template = _T("A");
        }
      p_texture = p_component_solid->GetShape().GetStl()->GetTextureFactory()->GetTextureWithName(_T("Label2"));
      if (p_texture)
        {
        MatAM::Label& label = p_component_solid->GetLabels().AddLabel(p_texture);
        label.GetParameters().m_mode_3d       = MatAM::LabelParameters::eEngraved;
        label.GetParameters().m_z[0]          = long(-0.50*StdBase::MInternalUnits::Get());
        label.GetParameters().m_z[1]          = long(+0.50*StdBase::MInternalUnits::Get());
        label.GetParameters().m_text_height   = long(4.0*StdBase::MInternalUnits::Get());
        label.GetParameters().m_text_template = _T("BC");
        }
      }

    if (true)
      {
      p_component_porous = p_assembly->AddComponent();
      p_component_porous->GetShape().SetStl(g_parts.stl_porous);
      p_component_porous->GetTypeInfo().SetType(MatAM::eSubComponentGeneric);
      p_component_porous->SetNameProcessParameters(_T("Porous"));

      MStlTexture* p_texture = p_component_porous->GetShape().GetStl()->GetTextureFactory()->GetTextureWithName(_T("Label"));
      if (p_texture)
        {
        MatAM::Label& label = p_component_porous->GetLabels().AddLabel(p_texture);
        label.GetParameters().m_mode_3d       = MatAM::LabelParameters::eSolid;
        label.GetParameters().m_z[0]          = long(-0.5*StdBase::MInternalUnits::Get());
        label.GetParameters().m_z[1]          = long(+0.0*StdBase::MInternalUnits::Get());
        label.GetParameters().m_text_height   = long(3.0*StdBase::MInternalUnits::Get());
        label.GetParameters().m_text_template = _T("Mizen");
        }

      MatAM::VolumeStructure& structure = p_component_porous->GetVolumeStructures().AddVolumeStructure();
      structure.SetEnabled(true);
      structure.SetUnitShape(g_parts.stl_cell_graph_normal);
      structure.GetUnitShape().SetIsSolid(false);
      //structure.GetUnitShape().SetThickness2D(long(0.5*StdBase::MInternalUnits::Get()));
      }
   
    }

  if (true)
    {
    MatSaxProjectWriter writer;

    writer.AddStl(&my_stl);
    writer.Write(g_parts.m_folder_out + _T("assembly_stl.mdck"));
    }

  if(true)
   {
    MatAM::ShapeDetailedProperties prop;
    prop.m_apply_labels           = true;
    prop.m_apply_volume_structure = true;

    p_component_base->GetShapeDetailed(prop);
    p_component_base->ReplaceShapeByDetailedShape();
    
    p_component_porous->GetShapeDetailed(prop);
    p_component_porous->ReplaceShapeByDetailedShape();
    }

  if (true)
    {
    MatAM::Assembly* p_assembly = MatAM::AssemblyStlDependency::GetInstance(&my_stl);

    MatAM::OperationStlExport stl_export;

    stl_export.SetAssembly(p_assembly);
    stl_export.SetFileNameSolid(_T("F:\\_Data\\__STL+\\stl_export.stl"));
    stl_export.SetFileNameFaces(_T("F:\\_Data\\__STL+\\stl_export_faces.stl"));
    stl_export.Operate();
    }

  if (true)
    {
    MatAM::Assembly* p_assembly = MatAM::AssemblyStlDependency::GetInstance(&my_stl);

    MStlBase                   detailed;
    MatAM::StlConvertAssembly  stl_assembly(p_assembly, NULL);

    stl_assembly.AddToStl(detailed);

    MatSaxProjectWriter writer;

    writer.AddStl(&detailed);
    writer.Write(g_parts.m_folder_out + _T("detailed.mdck"));
    }

  boost::shared_ptr<M2DSliceStack> p_stack = boost::shared_ptr<M2DSliceStack>(new M2DSliceStack);
  if (true)
    {
    MatAM::Assembly* p_assembly = MatAM::AssemblyStlDependency::GetInstance(&my_stl);

    MOperatorCreateSliceStack slicer;
    MatAM::SliceableAssembly  slice_assembly(p_assembly, NULL);
    M2DSliceZAreaTypeInfo     info;
    info.SetThickness(long(0.125*StdBase::MInternalUnits::Get()));

    slicer.SetSource(&slice_assembly);
    slicer.SetTarget(p_stack.get());
    slicer.SetZTypeInfo(info);
    slicer.Operate();

    MOperatorWriteSliceFile file_out;
    file_out.SetSource(p_stack.get());
    file_out.SetFileName((g_parts.m_folder_out + _T("slice_export.sli")).c_str());
    file_out.WriteFile(SLI_FORMAAT);

    MatSaxProjectWriter writer;

    p_stack->SetSliceThickness(long(0.125*StdBase::MInternalUnits::Get()));
    /*
    p_stack->GetProperties().SetProperty<long>(PropNameBeamCompensation, 0);
    MIndexedPropertySets& process_parameters = p_stack->GetProperties().AddProperty<MIndexedPropertySets>(_T("ProcessParameters"));

    process_parameters.GetPropertySet(-1).SetProperty<double>(_T("LaserSpeed"), double(100));
    process_parameters.GetPropertySet(-1).SetProperty<double>(_T("LaserFocus"), 0.1);
    
    MIndexedPropertySets::PropertySet* p_new_set = &process_parameters.AddPropertySet();
    p_new_set->SetProperty<double>(_T("LaserSpeed"), 125);
    p_new_set->SetProperty<double>(_T("LaserFocus"), 0.25);
    p_new_set = &process_parameters.AddPropertySet();
    p_new_set->SetProperty<double>(_T("LaserSpeed"), 250);
    p_new_set->SetProperty<double>(_T("LaserFocus"), 0.125);*/

    MatAM::SaxProjectObjectSliceStack sax_stack(p_stack.get(), false);
    writer.AddObject(&sax_stack);
    writer.Write(g_parts.m_folder_out + _T("stack_export.mdck"));
    }

  if (true)
    {
    /*
    MStlBase layered_stl;

    MatAM::OperationSliceContainerToLayeredStl layered_stl_operator;

    layered_stl_operator.SetStlOut(&layered_stl);
    layered_stl_operator.SetSlicesIn(p_stack.get());
    layered_stl_operator.Operate();

    MatSaxProjectWriter writer;
    writer.AddStl(&layered_stl);
    writer.Write(g_parts.m_folder_out + _T("layered_stl.mdck"));*/
    }


  if (true)
    {
    MatSaxProjectWriter writer;

    writer.AddStl(&my_stl);
    writer.Write(g_parts.m_folder_out + _T("assembly_stl_preprocessed.mdck"));
    }

  if (true)
    {
    MatSaxProjectReader reader;
    reader.Read(_T("F:\\_Data\\__STL+\\assembly_stl.mdck"));
    }

  if (true)
    {
    MatAM::SaxProjectObjectHandlerSliceStack stack_handler;
    MatSaxProjectReader reader;
    reader.AddHandler(&stack_handler);
    reader.Read(g_parts.m_folder_out + _T("stack_export.mdck"));

    std::vector<MatAM::hSaxProjectObjectSliceStack> stacks;
    reader.GetObjects(stacks);
    MatAM::SaxProjectObjectSliceStack* p_sax_stack = stacks[0].get();

    CHECK(p_sax_stack->GetStack()->GetProperties() == p_stack->GetProperties());
    }

  if (true)
    {
    MatAM::Platform platform;

    platform.GetMachine()->m_platform.envelope.dx = long(250*StdBase::MInternalUnits::Get());
    platform.GetMachine()->m_platform.envelope.dy = long(200*StdBase::MInternalUnits::Get());
    platform.GetMachine()->m_platform.envelope.dz = long(150*StdBase::MInternalUnits::Get());
    platform.GetMachine()->SetName(_T("MyDummyMachine"));
    platform.GetMachine()->SetMachineClass(_T("MyDummyMachineClass"));
    platform.GetMachine()->commentaar = _T("MyDummyComment");

    MatAM::PartInstance* p_instance1 = platform.AddPartInstance(&my_stl);
    MatAM::PartInstance* p_instance2 = platform.AddPartInstance(&my_stl);
    MatAM::PartInstance* p_instance3 = platform.AddPartInstance(&my_stl);
    MatAM::PartInstance* p_instance4 = platform.AddPartInstance(&my_stl);

    p_instance2->GetPosition().MakeTranslation(MMPoint3dDouble(125*StdBase::MInternalUnits::Get(),
                                                                 0*StdBase::MInternalUnits::Get(),
                                                                 0*StdBase::MInternalUnits::Get()));
    p_instance3->GetPosition().MakeTranslation(MMPoint3dDouble(  0*StdBase::MInternalUnits::Get(),
                                                               125*StdBase::MInternalUnits::Get(),
                                                                 0*StdBase::MInternalUnits::Get()));
    p_instance4->GetPosition().MakeTranslation(MMPoint3dDouble(125*StdBase::MInternalUnits::Get(),
                                                               125*StdBase::MInternalUnits::Get(),
                                                                 0*StdBase::MInternalUnits::Get()));
    

    p_instance1->GetProperties().SetProperty<std::wstring>(_T("A"), _T("W"));
    p_instance2->GetProperties().SetProperty<std::wstring>(_T("A"), _T("X"));
    p_instance3->GetProperties().SetProperty<std::wstring>(_T("A"), _T("Y"));
    p_instance4->GetProperties().SetProperty<std::wstring>(_T("A"), _T("Z"));
    p_instance4->GetProperties().SetProperty<std::wstring>(_T("Mizen"), _T("Materialise"));
    
    CHECK(p_instance1->IsValid());
    CHECK(platform.AllInstancesAreValid());

    if (false)
      {
      // make sure we have the right parameters sets
      FillConceptLaserParameters(platform.GetMachine()->GetCLParameters());

      M2DSliceStack stack;
      MOperatorCreateSliceStack slicer;
      MatAM::SliceablePlatform  slice_platform(&platform);
      M2DSliceZAreaTypeInfo     info;
      info.SetThickness(long(0.125*StdBase::MInternalUnits::Get()));

      slicer.SetSource(&slice_platform);
      slicer.SetTarget(&stack);
      slicer.SetZTypeInfo(info);
      slicer.Operate();

      MatSaxProjectWriter writer;

      stack.SetSliceThickness(long(0.125*StdBase::MInternalUnits::Get()));

      MatAM::SaxProjectObjectSliceStack sax_stack(&stack, false);
      writer.AddObject(&sax_stack);
      writer.Write(g_parts.m_folder_out + _T("platform_stack_export.mdck"));
      }
    if (true)
      {
      MStlBase                   stl_platform;
      MatAM::StlConvertPlatform  stlconvert_platform(&platform);

      stlconvert_platform.AddToStl(stl_platform, NULL);

      MatSaxProjectWriter writer;

      writer.AddStl(&stl_platform);
      writer.Write(g_parts.m_folder_out + _T("platform_stl.mdck"));
      }
    }
  }

///////////////////////////////////////////////////////////////////////////////
