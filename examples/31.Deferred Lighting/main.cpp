#include <irrlicht.h>
#include <iostream>
#include <driverChoice.h>
#include <exampleHelper.h>

using namespace irr;

#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

IrrlichtDevice *device = 0;

int main()
{
	device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(640, 480));
	if (device == 0)
	{
		return 1;
	}

	video::IVideoDriver *driver = device->getVideoDriver();
	scene::ISceneManager *smgr = device->getSceneManager();
	gui::IGUIEnvironment *gui = device->getGUIEnvironment();

	const io::path mediaPath = getExampleMediaPath();

	//添加天空盒
	auto *skyBox = smgr->addSkyBoxSceneNode(
		driver->getTexture(mediaPath + "irrlicht2_up.jpg"),
		driver->getTexture(mediaPath + "irrlicht2_dn.jpg"),
		driver->getTexture(mediaPath + "irrlicht2_lf.jpg"),
		driver->getTexture(mediaPath + "irrlicht2_rt.jpg"),
		driver->getTexture(mediaPath + "irrlicht2_ft.jpg"),
		driver->getTexture(mediaPath + "irrlicht2_bk.jpg")
	);
	//driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

	//添加模型
	auto *node = smgr->addMeshSceneNode(smgr->getMesh(mediaPath + "man.obj"));
	if (node)
	{
		node->setPosition(core::vector3df(0, 0, 0));
		node->setScale(core::vector3df(100, 100, 100));
	}
	
	//添加灯光
	auto *light = smgr->addLightSceneNode(0, core::vector3df(115, 5, -105),
		video::SColorf(1.0f, 1.0f, 1.0f));
	light->setRadius(100000);

	// set ambient light
	smgr->setAmbientLight(video::SColor(0, 60, 60, 60));
	
	//添加相机
	auto *cam = smgr->addCameraSceneNodeMaya();
	cam->setPosition(core::vector3df(-100, 50, 100));
	cam->setTarget(core::vector3df(0,0,0));

	int lastFPS = -1;
	while (device->run())
	{
		//if (device->isWindowActive())
		{
			driver->beginScene(video::ECBF_COLOR | video::ECBF_DEPTH, video::SColor(255, 0, 0, 0));
			smgr->drawAll();
			driver->endScene();

			int fps = driver->getFPS();
			if (lastFPS != fps)
			{
				device->setWindowCaption(core::stringw(fps).c_str());
			}
		}
	}
	device->drop();

	return 0;
}