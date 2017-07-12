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

	struct MD5Header
	{
		unsigned int version;
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

		void loadMeshFile();

		void loadAnimFile();

		void parseHeader();

		bool parseSection(Section &sec);

		void parseMesh();

		void parseMesh(const c8 *buf);

		void _parseShader(const c8 *bufBegin, const c8 *bufEnd);
		void _parseVerts(const c8 *bufBegin, const c8 *bufEnd);
		void _parseTris(const c8 *bufBegin, const c8 *bufEnd);
		void _parseWeights(const c8 *bufBegin, const c8 *bufEnd);

	protected:
		bool skipSpacesAndLineEnd();
		bool skipLine();
		bool parseString(const c8 ** buf, int lineNumber, core::stringc &name);
		bool parseVec(const c8 **buf, int lineNumber, core::vector3df &vec);

		void convertVecToQuat(core::vector3df &vec, core::quaternion &quat);

	public:
		SectionList	m_sections;

	private:
		CSkinnedMesh *AnimatedMesh;

		video::IVideoDriver *Driver;

		MD5Header m_header;

		c8 *m_buffer = NULL;
		int m_fileSize = 0;
		int m_lineNumber = 0;
		core::stringc m_fileName;
	};
}
}

#endif
