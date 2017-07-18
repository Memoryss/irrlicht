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

	//������Ϣ
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

		//���ؼ�������
		void processMesh(fbxsdk::FbxNode *pNode);

		//���ع���
		void processSkeleton(fbxsdk::FbxNode *pNode);

		//���صƹ�
		void processLight(fbxsdk::FbxNode *pNode);

		//�������
		void processCamera();

		//���ز���
		void loadMaterial();

		//
		core::vector3df readMaterialColor(fbxsdk::FbxPropertyT<FbxDouble3> colorProperty, fbxsdk::FbxPropertyT<FbxDouble> colorFactorProperty);



		//���ز�������
		void loadMaterialAttribute(fbxsdk::FbxSurfaceMaterial *pMaterial);

		//����������Ϣ
		void loadMaterialTexture(fbxsdk::FbxSurfaceMaterial *pMaterial);

		//��ȡ����
		void readVertex(fbxsdk::FbxMesh *pMesh, int vIndex);

		//��ȡ��ɫ��Ϣ
		void readColor(fbxsdk::FbxMesh *pMesh, int vIndex, int cIndex);

		//��ȡuv
		void readUV(fbxsdk::FbxMesh* pMesh, int vIndex, int uvIndex, int uvLayer);

		//��ȡnormal
		void readNormal(fbxsdk::FbxMesh *pMesh, int vIndex, int cIndex);

		//��ȡTangent
		void readTangent(fbxsdk::FbxMesh *pMesh, int vIndex, int cIndex);

	private:
		fbxsdk::FbxScene *m_fbxScene;
		fbxsdk::FbxManager *m_fbxManager;

		MaterialList m_materials;
	};
}
}

#endif
