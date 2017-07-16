#include "CFBXMeshFileLoader.h"

#include <math.h>
#include <assert.h>

#include "CSkinnedMesh.h"
#include "CMeshTextureLoader.h"
#include "IReadFile.h"
#include "IFileSystem.h"
#include "ISceneManager.h"
#include "coreutil.h"
#include "fast_atof.h"
#include "os.h"

#ifdef _WIN32
	#ifdef _DEBUG
		#pragma comment(lib, "FBX/2016.1.2/lib/vs2015/x86/debug/libfbxsdk.lib")
	#else
		#pragma comment(lib, "FBX/2016.1.2/lib/vs2015/x86/release/libfbxsdk.lib")
	#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib, "FBX/2016.1.2/lib/vs2015/x64/debug/libfbxsdk.lib")
	#else
		#pragma comment(lib, "FBX/2016.1.2/lib/vs2015/x64/release/libfbxsdk.lib")
	#endif
#endif


namespace irr
{
namespace scene
{
	CFBXMeshFileLoader::CFBXMeshFileLoader(scene::ISceneManager * smgr, io::IFileSystem * fs)
	{
		#ifdef _DEBUG
		setDebugName("CFBXMeshFileLoader");
		#endif

		m_fbxManager = fbxsdk::FbxManager::Create();
		if (!m_fbxManager)
		{
			os::Printer::log("Error: Unable to create FBX Manager!", ELL_ERROR);
			assert(true);
		}
		os::Printer::log("Autodesk FBX SDK version ", m_fbxManager->GetVersion(), ELL_DEBUG);

		fbxsdk::FbxIOSettings *ios = fbxsdk::FbxIOSettings::Create(m_fbxManager, IOSROOT);
		m_fbxManager->SetIOSettings(ios);

		m_fbxScene = fbxsdk::FbxScene::Create(m_fbxManager, "");
		if (!m_fbxScene)
		{
			os::Printer::log("Error: Unable to create FBX Scene!", ELL_ERROR);
			assert(true);
		}

		TextureLoader = new CMeshTextureLoader(fs, smgr->getVideoDriver());
	}

	CFBXMeshFileLoader::~CFBXMeshFileLoader()
	{
		if (m_fbxManager)
		{
			m_fbxManager->Destroy();
		}

		if (m_fbxScene)
		{
			m_fbxScene->Destroy(true);
		}
	}

	bool CFBXMeshFileLoader::isALoadableFileExtension(const io::path& filename) const
	{
		return core::hasFileExtension(filename, "fbx");
	}


	IAnimatedMesh * CFBXMeshFileLoader::createMesh(io::IReadFile *file)
	{
		fbxsdk::FbxImporter *importer = fbxsdk::FbxImporter::Create(m_fbxManager, "");
		bool bRes = importer->Initialize(file->getFileName().c_str(), -1, m_fbxManager->GetIOSettings());
		if (!bRes)
		{
			os::Printer::log("Error: can't load file, name:", file->getFileName().c_str(), ELL_ERROR);
			return NULL;
		}

		bRes = importer->Import(m_fbxScene);
		if (!bRes)
		{
			os::Printer::log("Could not parse FBX file, name:", file->getFileName().c_str(), ELL_ERROR);
			return NULL;
		}

		int lFileMajor, lFileMinor, lFileRevision;
		importer->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);
		core::stringc temp(file->getFileName());
		temp.append(" is ");
		temp.append(lFileMajor);
		temp.append(".");
		temp.append(lFileMinor);
		temp.append(".");
		temp.append(lFileRevision);

		os::Printer::log("FBX file format version for file, name:", temp.c_str(), ELL_DEBUG);

		//m_fbxTransform.in
	}

}
}

