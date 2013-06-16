#include "TestUtilities.h"

#include "..\Platforms\Platform.h"
#include "..\Platforms\PartInstance.h"
#include "..\Platforms\SaxProjectObjectPlatform.h"
#include "..\Slicer\SliceablePlatform.h"
#include "..\SliceStack\SaxProjectObjectSliceStack.h"

#include <MatUnitTesting/MatUnitTesting.h>
#include <MatBase\MFile.h>
#include <MatBase\TimeCount.h>

#include <Mat2DSlices\M2DSliceStack.h>

#include <MatStl\stl_base.h>
#include <MatStl\MatSaxProject\MatSaxProjectReader.h>
#include <MatStl\MatSaxProject\MatSaxProjectWriter.h>

#include <MatStlSlicer\MOperatorCreateSliceStack.h>

#include <MatMachines\MSAXMachine.h>

using namespace MatAM;

///////////////////////////////////////////////////////////////////////////////

SUITE(SuiteTestPlatforms)
  {
  TEST(TestPlatforms1)
    {
    return;
    g_parts.Load();

    Platform platform;
    MStlBase stl(g_parts.stl_total);

    platform.GetMachine()->m_platform.envelope.dx = long(250*StdBase::MInternalUnits::Get());
    platform.GetMachine()->m_platform.envelope.dy = long(200*StdBase::MInternalUnits::Get());
    platform.GetMachine()->m_platform.envelope.dz = long(150*StdBase::MInternalUnits::Get());
    platform.GetMachine()->SetName(_T("MyDummyMachine"));
    platform.GetMachine()->SetMachineClass(_T("MyDummyMachineClass"));
    platform.GetMachine()->commentaar = _T("MyDummyComment");

    if (true)
      {
      PartInstance* p_instance1 = platform.AddPartInstance(&stl);
      PartInstance* p_instance2 = platform.AddPartInstance(&stl);
      PartInstance* p_instance3 = platform.AddPartInstance(&stl);
      PartInstance* p_instance4 = platform.AddPartInstance(&stl);

      MMTransformMatrix translation;
      translation.MakeTranslation(MMPoint3dDouble(250*StdBase::MInternalUnits::Get(),
                                                    0*StdBase::MInternalUnits::Get(),
                                                    0*StdBase::MInternalUnits::Get()));
      p_instance3->SetPosition(translation);
      translation.MakeTranslation(MMPoint3dDouble(  0*StdBase::MInternalUnits::Get(),
                                                  250*StdBase::MInternalUnits::Get(),
                                                    0*StdBase::MInternalUnits::Get()));
      p_instance4->SetPosition(translation);
      
      CHECK(p_instance1->IsValid());
      CHECK(platform.AllInstancesAreValid());

      platform.RemovePartInstance(p_instance2);
      }
    //CHECK(!platform.AllInstancesAreValid());

    if (true)
      {
      MatSaxProjectWriter writer;

      writer.AddStl(&stl);
      SaxProjectObjectPlatform sax_platform(&platform, false);
      writer.AddObject(&sax_platform);
      writer.Write(g_parts.m_folder_out + _T("platform.mdck"));
      }
    if (true)
      {
      SaxProjectObjectHandlerPlatform platform_handler;
      MatSaxProjectReader reader;
      reader.AddHandler(&platform_handler);
      reader.Read(g_parts.m_folder_out + _T("platform.mdck"));
      }
    if (true)
      {
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
    }
  }

///////////////////////////////////////////////////////////////////////////////

