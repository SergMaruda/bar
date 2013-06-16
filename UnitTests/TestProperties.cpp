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
#include <Mat2DSlices\Properties\PersistentProperty.h>
#include <Mat2DSlices\Properties\IndexedPropertySets.h>
#include <Mat2DSlices\Properties\IndexedProperties.h>

#include <MatStl\stl_base.h>
#include <MatStl\MatSaxProject\MatSaxProjectReader.h>
#include <MatStl\MatSaxProject\MatSaxProjectWriter.h>

#include <MatStlSlicer\MOperatorCreateSliceStack.h>

#include <MatMachines\MSAXMachine.h>

using namespace MatAM;

///////////////////////////////////////////////////////////////////////////////

SUITE(SuiteTestProperties)
  {
  TEST(TestProperties)
    {
    g_parts.Load();

    Platform platform;
    MStlBase stl;

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
    
    if (true)
      {
      platform.GetProperties().SetProperty<long>(_T("MyProperty"), 2000);
      platform.GetProperties().SetProperty<bool>(_T("MyPropertyBool"), true);
      platform.GetProperties().SetProperty<std::wstring>(_T("MyPropertyString"), _T("MyString"));

      PersistentProperties& sub_properties = platform.GetProperties().AddProperty<PersistentProperties>(_T("MySubPropertyString"));
      sub_properties.SetProperty<long>(_T("MySubProperty"), 25000);
      sub_properties.SetProperty<bool>(_T("MySubPropertyBool"), false);
      sub_properties.SetProperty<std::wstring>(_T("MySubPropertyString"), _T("MySecondString"));

      typedef IndexedProperties<PersistentProperties>  IndexedProps;

      IndexedProps& props = platform.GetProperties().AddProperty<IndexedProps>(_T("IndexedProps"));
      props.GetProperty().SetProperty<double>(_T("LaserSpeed"), 100);
      props.GetProperty().SetProperty<double>(_T("LaserFocus"), 0.1);
      IndexedPropertySets::Property* p_new_props = &props.AddProperty();
      p_new_props->SetProperty<double>(_T("LaserSpeed"), 125);
      p_new_props->SetProperty<double>(_T("LaserFocus"), 0.25);
      p_new_props = &props.AddProperty();
      p_new_props->SetProperty<double>(_T("LaserSpeed"), 250);
      p_new_props->SetProperty<double>(_T("LaserFocus"), 0.125);
      p_new_props->SetProperty<double>(_T("LaserFocus"), 0.250);
      p_new_props->SetProperty<long>(_T("LaserFocus"), 20);
      p_new_props->RemoveProperty(_T("LaserFocus"));
      p_new_props->SetProperty<double>(_T("LaserFocus"), 0.250);
      p_new_props->SetProperty<long>(_T("LaserFocus"), 20);
      }

    if (true)
      {
      MatSaxProjectWriter writer;

      writer.AddStl(&stl);
      SaxProjectObjectPlatform sax_platform(&platform, false);
      writer.AddObject(&sax_platform);
      writer.Write(g_parts.m_folder_out + _T("properties.mdck"));
      }
    if (true)
      {
      SaxProjectObjectHandlerPlatform platform_handler;
      MatSaxProjectReader reader;
      reader.AddHandler(&platform_handler);
      reader.Read(g_parts.m_folder_out + _T("properties.mdck"));
      std::vector<hSaxProjectObjectPlatform> platforms;
      
      reader.GetObjects<SaxProjectObjectPlatform>(platforms);

      CHECK(platforms.size() == 1);
      CHECK(platform.GetProperties() == platforms[0]->GetPlatform()->GetProperties());
      }
    }
  }

///////////////////////////////////////////////////////////////////////////////

