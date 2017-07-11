#include "CMD5MeshFileLoader.h"

#include "CSkinnedMesh.h"
#include "IReadFile.h"
#include "coreutil.h"
#include "os.h"

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

		if (!loadFile(file)) 
		{
			return false;
		}

		AnimatedMesh = new CSkinnedMesh();
	}

	bool CMD5MeshFileLoader::loadFile(io::IReadFile *file) {
		int fileSize = file->getSize();
		if (fileSize <= 0)
		{
			os::Printer::log("file size < 0. name:", file->getFileName().c_str(), ELL_ERROR);
			return false;
		}

		c8 *buf = new c8[fileSize];
		memset(buf, 0, fileSize);
		file->read(buf, fileSize);
		const c8 *bufEnd = buf + fileSize;

		_parseHeader(buf, bufEnd);

		if (core::hasFileExtension(file->getFileName(), "md5mesh")) 
		{
			loadMeshFile(buf, bufEnd);
		}
		else 
		{
			loadAnimFile(buf, bufEnd);
		}

		return true;
	}

	void CMD5MeshFileLoader::loadMeshFile(const c8 *bufBegin, const c8 *bufEnd)
	{

	}

	void CMD5MeshFileLoader::loadAnimFile(const c8 *bufBegin, const c8 *bufEnd)
	{

	}

	void CMD5MeshFileLoader::parseJoints(const c8 *bufBegin, const c8 *bufEnd)
	{

	}

	void CMD5MeshFileLoader::parseMesh(const c8 *bufBegin, const c8 *bufEnd)
	{

	}

	const c8* CMD5MeshFileLoader::_parseHeader(const c8 *bufBegin, const c8 *bufEnd, bool isMesh)
	{
		auto &header = isMesh ? m_meshHeader : m_animHeader;
		const c8 *p = strstr(bufBegin, "MD5Version");
		p += 10;
		while (p < bufEnd)
		{
			while (*p == ' ') {
				++p;
			}

			const c8 *p1 = p;
			while ((*p1 != ' ' || *p1 != '\n') && p < bufEnd) {
				++p1;
			}

			if (p < bufEnd)
			{
			}

			core::stringc versionStr(p, (p1 - p));
			header.version = atoi(versionStr.c_str());

			p = strstr(p1, "commandline");

			header.commondLineStr =
		}
	}

	void CMD5MeshFileLoader::_parseShader(const c8 * bufBegin, const c8 * bufEnd)
	{

	}

	void CMD5MeshFileLoader::_parseVerts(const c8 *bufBegin, const c8 *bufEnd)
	{

	}

	void CMD5MeshFileLoader::_parseTris(const c8 *bufBegin, const c8 *bufEnd)
	{

	}

	void CMD5MeshFileLoader::_parseWeights(const c8 *bufBegin, const c8 *bufEnd)
	{

	}

}
}

