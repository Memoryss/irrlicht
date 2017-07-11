#include "CMD5MeshFileLoader.h"

#include "CSkinnedMesh.h"
#include "coreutil.h"

namespace irr
{
namespace scene
{

	CMD5MeshFileLoader::CMD5MeshFileLoader(video::IVideoDriver *dirver) : Driver(dirver), AnimatedMesh(NULL)
	{
	#ifdef _DEBUG
		setDebugName("CMS3DMeshFileLoader");
	#endif
	}

	bool CMD5MeshFileLoader::isALoadableFileExtension(const io::path& filename) const
	{
		return core::hasFileExtension(filename, "md5mesh", "md5anim");
	}

	IAnimatedMesh* CMD5MeshFileLoader::createMesh(io::IReadFile* file)
	{
		if (file) 
		{
			return NULL;
		}

		AnimatedMesh = new CSkinnedMesh();
	}
	void CMD5MeshFileLoader::loadFile(io::IReadFile file, CSkinnedMesh * mesh)
	{
	}
}
}

