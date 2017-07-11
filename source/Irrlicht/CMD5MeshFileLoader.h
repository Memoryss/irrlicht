#ifndef __C_FBX_MESH_FILE_LOADER_H_INCLUDE__
#define __C_FBX_MESH_FILE_LOADER_H_INCLUDE__

#include "IMeshLoader.h"
#include "irrString.h"

namespace irr
{
	namespace video
	{
		class IVideoDriver;
	}
	
	namespace io
	{
		class IReadFile;
	}

namespace scene
{
	class CSkinnedMesh;

	struct MD5Header
	{
		int version;
		core::stringc commondLineStr;
	};

	class CMD5MeshFileLoader : public IMeshLoader
	{
	public:
		CMD5MeshFileLoader(video::IVideoDriver *dirver);

		virtual bool isALoadableFileExtension(const io::path& filename) const override;


		virtual IAnimatedMesh* createMesh(io::IReadFile* file) override;

	protected:
		bool loadFile(io::IReadFile *file);

		void loadMeshFile(const c8 *bufBegin, const c8 *bufEnd);

		void loadAnimFile(const c8 *bufBegin, const c8 *bufEnd);

		void parseJoints(const c8 *bufBegin, const c8 *bufEnd);

		void parseMesh(const c8 *bufBegin, const c8 *bufEnd);

		const c8* _parseHeader(const c8 *bufBegin, const c8 *bufEnd, bool isMesh);
		void _parseShader(const c8 *bufBegin, const c8 *bufEnd);
		void _parseVerts(const c8 *bufBegin, const c8 *bufEnd);
		void _parseTris(const c8 *bufBegin, const c8 *bufEnd);
		void _parseWeights(const c8 *bufBegin, const c8 *bufEnd);

	private:
		CSkinnedMesh *AnimatedMesh;

		video::IVideoDriver *Driver;

		MD5Header m_meshHeader;
		MD5Header m_animHeader;
	};
}
}

#endif
