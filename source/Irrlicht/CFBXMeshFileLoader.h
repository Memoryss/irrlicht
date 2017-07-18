#ifndef __C_FBX_MESH_FILE_LOADER_H_INCLUDE__
#define __C_FBX_MESH_FILE_LOADER_H_INCLUDE__

#include <unordered_map>

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

	//材质信息
	struct MaterialData
	{
		core::stringc mName;
		core::vector3df mAmbient;
		core::vector3df mDiffuse;
		core::vector3df mSpecular;
		core::vector3df mEmissive;
		float mPower;
		
		std::unordered_map<core::stringc, core::stringc> mTextures;
	};

	typedef core::array<MaterialData> MaterialList;

	class CFBXMeshFileLoader : public IMeshLoader
	{
	public:
		CFBXMeshFileLoader(scene::ISceneManager *smgr, io::IFileSystem *fs);

		virtual ~CFBXMeshFileLoader();

		virtual bool isALoadableFileExtension(const io::path& filename) const override;

		virtual IAnimatedMesh * createMesh(io::IReadFile *file) override;

	protected:
		void processNode(fbxsdk::FbxNode *pNode);

		//加载几何网格
		void processMesh(fbxsdk::FbxNode *pNode);

		//加载骨骼
		void processSkeleton(fbxsdk::FbxNode *pNode);

		//加载灯光
		void processLight(fbxsdk::FbxNode *pNode);

		//加载相机
		void processCamera();

		//加载材质
		void loadMaterial();

		//
		core::vector3df readMaterialColor(fbxsdk::FbxPropertyT<FbxDouble3> colorProperty, fbxsdk::FbxPropertyT<FbxDouble> colorFactorProperty);



		//加载材质属性
		void loadMaterialAttribute(fbxsdk::FbxSurfaceMaterial *pMaterial);

		//加载纹理信息
		void loadMaterialTexture(fbxsdk::FbxSurfaceMaterial *pMaterial);

		//读取顶点
		void readVertex(fbxsdk::FbxMesh *pMesh, int vIndex);

		//读取颜色信息
		void readColor(fbxsdk::FbxMesh *pMesh, int vIndex, int cIndex);

		//读取uv
		void readUV(fbxsdk::FbxMesh* pMesh, int vIndex, int uvIndex, int uvLayer);

		//读取normal
		void readNormal(fbxsdk::FbxMesh *pMesh, int vIndex, int cIndex);

		//读取Tangent
		void readTangent(fbxsdk::FbxMesh *pMesh, int vIndex, int cIndex);

	private:
		fbxsdk::FbxScene *m_fbxScene;
		fbxsdk::FbxManager *m_fbxManager;

		MaterialList m_materials;
	};
}
}

#endif
