#ifndef __C_FBX_MESH_FILE_LOADER_H_INCLUDE__
#define __C_FBX_MESH_FILE_LOADER_H_INCLUDE__

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

	struct Element
	{
		c8 * szStart;
		unsigned int iLineNumber;
	};

	typedef core::array<Element> ElementList;

	struct Section
	{
		unsigned int iLineNumber;

		ElementList mElements;

		core::stringc mName;

		core::stringc mGlobalValue;
	};

	typedef core::array<Section> SectionList;

	struct MD5Vetex
	{
		core::vector2df mUV;
		unsigned int mFirstWeight = 0;
		unsigned int mNumWeights = 0;
	};

	typedef core::array<MD5Vetex> VertexList;

	struct MD5Weight
	{
		unsigned int mBone = 0;

		float mWeight = 0.0f;

		core::vector3df mOffsetPosition;

		unsigned int mVertexID = 0;
	};

	typedef core::array<MD5Weight> WeightList;

	struct MD5Face
	{
		unsigned int mNumIndices = 0;

		core::vector3di mIndices;
	};

	typedef core::array<MD5Face> FaceList;

	struct MD5Header
	{
		unsigned int version;
		core::stringc commondLineStr;
	};

	struct MD5Mesh
	{
		WeightList mWeights;
		VertexList mVertices;
		FaceList mFaces;

		core::stringc mShader;
	};

	typedef core::array<MD5Mesh> MeshList;

	struct MD5BaseJoint
	{
		core::stringc mName;

		int mParentIndex;
	};

	struct MD5Bone : public MD5BaseJoint
	{
		core::vector3df mPositionXYZ;

		core::vector3df mRotationQuat;
		core::quaternion mRotationQuatConverted;

		//core::matrix4 mTransform;
		//core::matrix4 mInvTransform;

		//unsigned int mMap;
	};

	typedef core::array<MD5Bone> BoneList;

	struct AnimBone : public MD5BaseJoint
	{
		u32 iFlags;
		u32 iFirstKeyIndex;
	};

	typedef core::array<AnimBone> AnimBoneList;

	struct BaseFrame
	{
		core::vector3df vPositionXYZ;
		core::vector3df vRotationQuat;
	};

	typedef core::array<BaseFrame> BaseFrameList;

	struct CameraAnimFrame : public BaseFrame
	{
		float fFOV;
	};

	typedef core::array<CameraAnimFrame> CameraFrameList;

	struct Frame
	{
		u32 iIndex;
		core::array<float> mValues;
	};

	typedef core::array<Frame> FrameList;

	class CMD5MeshFileLoader : public IMeshLoader
	{
	public:
		CMD5MeshFileLoader(scene::ISceneManager* smgr, io::IFileSystem* fs);

		virtual bool isALoadableFileExtension(const io::path& filename) const override;


		virtual IAnimatedMesh* createMesh(io::IReadFile* file) override;

	protected:
		bool loadMeshFile(io::IReadFile *file);

		bool loadAnimFile(io::IReadFile *file);

		bool loadFileImp(io::IReadFile *file);

		void parseHeader();

		bool parseSection(Section &sec);

		void parseMesh();

		void parseAnim();

	protected:
		bool skipSpacesAndLineEnd();
		bool skipLine();
		bool parseString(const c8 ** buf, int lineNumber, core::stringc &name);
		bool parseVec(const c8 **buf, int lineNumber, core::vector3df &vec);

		void convertVecToQuat(const core::vector3df &vec, core::quaternion &quat);

	public:
		SectionList	m_sections;

	private:
		CSkinnedMesh *AnimatedMesh;

		ISceneManager* SceneManager;
		io::IFileSystem* FileSystem;

		MD5Header m_header;

		c8 *m_buffer = NULL;
		int m_fileSize = 0;
		int m_lineNumber = 0;
		core::stringc m_fileName;

		//mesh
		MeshList m_meshs;
		BoneList m_joints;

		//anim
		float m_frameRate;
		AnimBoneList m_animatedBones;
		BaseFrameList m_baseFrames;
		FrameList m_frames;
		u32 m_numAnimatedComponents;
	};
}
}

#endif
