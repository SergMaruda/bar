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
#include <Mat2DSlices\Properties\PersistentProperties.h>

#include <MatStl\stl_base.h>
#include <MatStl\MatSaxProject\MatSaxProjectReader.h>
#include <MatStl\MatSaxProject\MatSaxProjectWriter.h>

#include <MatStlSlicer\MOperatorCreateSliceStack.h>
#include <MatStlSlicer\MOperatorCreateSliceStack.h>
#include <MatStlSlicer\ConceptLaserParameters.h>

#include <MatMachines\MSAXMachine.h>

using namespace MatAM;

///////////////////////////////////////////////////////////////////////////////

SUITE(SuiteTestConceptLaser)
  {
  TEST(TestCL)
    {
    //return;
    Platform platform;
    MStlBase stl;

    platform.GetMachine()->m_platform.envelope.dx = long(250*StdBase::MInternalUnits::Get());
    platform.GetMachine()->m_platform.envelope.dy = long(200*StdBase::MInternalUnits::Get());
    platform.GetMachine()->m_platform.envelope.dz = long(150*StdBase::MInternalUnits::Get());
    platform.GetMachine()->SetName(_T("MyDummyMachine"));
    platform.GetMachine()->SetMachineClass(_T("MyDummyMachineClass"));
    platform.GetMachine()->commentaar = _T("MyDummyComment");

    MCLParameters&    cl_parameters = platform.GetMachine()->GetCLParameters();

    FillConceptLaserParameters(cl_parameters);

    cl_parameters.Save(g_parts.m_folder_out + _T("CLParameters.xml"));

    MCLParameters cl_parameters2;
    cl_parameters2.Load(g_parts.m_folder_out + _T("CLParameters.xml"));

    platform.GetProperties().SetProperty<MCLParameters>(_T("CLParameters"), cl_parameters);

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
    }
  }

///////////////////////////////////////////////////////////////////////////////

