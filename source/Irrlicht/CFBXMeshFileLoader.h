#ifndef __C_FBX_MESH_FILE_LOADER_H_INCLUDE__
#define __C_FBX_MESH_FILE_LOADER_H_INCLUDE__

#include <fbxsdk.h>

#include "IMeshLoader.h"
#include "irrString.h"
#include "irrArray.h"
#include "vector2d.h"
#include "vector3d.h"
#include "quaternion.h"

namespace irr
{
	namespace video
	{
		class IVideoDriver;
	}
	
	namespace io
	{
		class IReadFile;
		class IFileSystem;
	}

namespace scene
{
	class ISceneManager;
	
	class CSkinnedMesh;

	class CFBXMeshFileLoader : public IMeshLoader
	{
	public:
		CFBXMeshFileLoader(scene::ISceneManager *smgr, io::IFileSystem *fs);

		virtual ~CFBXMeshFileLoader();

		virtual bool isALoadableFileExtension(const io::path& filename) const override;

		virtual IAnimatedMesh * createMesh(io::IReadFile *file) override;

	private:
		fbxsdk::FbxScene *m_fbxScene;
		fbxsdk::FbxManager *m_fbxManager;
		//fbxsdk::FBXTransformer m_fbxTransform;
	};
}
}

#endif
