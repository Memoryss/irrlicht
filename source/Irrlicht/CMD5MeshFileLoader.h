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
	

namespace scene
{
	class CSkinnedMesh;

	struct MD5MeshHeader
	{
		int version;
		core::string command
	};

	class CMD5MeshFileLoader : public IMeshLoader
	{
	public:
		CMD5MeshFileLoader(video::IVideoDriver *dirver);

		virtual bool isALoadableFileExtension(const io::path& filename) const override;


		virtual IAnimatedMesh* createMesh(io::IReadFile* file) override;

	protected:
		void loadFile(io::IReadFile file, CSkinnedMesh *mesh);

	private:
		CSkinnedMesh *AnimatedMesh;

		video::IVideoDriver *Driver;
	};
}
}

#endif
