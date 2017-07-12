#include "CMD5MeshFileLoader.h"

#include <math.h>

#include "CSkinnedMesh.h"
#include "IReadFile.h"
#include "coreutil.h"
#include "fast_atof.h"
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

		os::Printer::log("parse md5 base file begin. name:", file->getFileName().c_str(), ELL_DEBUG);

		c8 *buf = new c8[fileSize];
		memset(buf, 0, fileSize);
		file->read(buf, fileSize);

		m_buffer = buf;
		m_fileSize = fileSize;
		m_fileName = file->getFileName();
		m_lineNumber = 0;

		parseHeader();
		while (true)
		{
			m_sections.push_back(Section());
			auto &sec = m_sections.getLast();
			if (!parseSection(sec))
			{
				break;
			}
		}

		os::Printer::log("parse md5 base file end. name:", file->getFileName().c_str(), ELL_DEBUG);

		return true;
	}

	void CMD5MeshFileLoader::parseHeader()
	{
		//解析version
		core::skipSpace(&m_buffer);
		if (!core::tokenMatch(&m_buffer, "MD5Version", 10))
		{
			os::Printer::log("Invalid MD5 file: MD5Version tag has not been found", m_fileName.c_str(), ELL_ERROR);
			return;
		}

		core::skipSpace(&m_buffer);
		m_header.version = core::strtoul10(m_buffer, &m_buffer);
		if (m_header.version != 10)
		{
			os::Printer::log("MD5 version tag is unknown (10 is expected)", m_fileName.c_str(), ELL_ERROR);
			return;
		}

		//解析cmd
		skipLine();
		core::skipSpace(&m_buffer);
		if (!core::tokenMatch(&m_buffer, "commandline", 13))
		{
			os::Printer::log("Invalid MD5 file: commandline tag has not been found ", m_fileName.c_str(), ELL_ERROR);
			return;
		}

		core::skipSpace(&m_buffer);
		if (!parseString(&m_buffer, m_lineNumber, m_header.commondLineStr))
		{
			os::Printer::log("Invalid MD5 file: ' \" ' tag has not been found ", m_fileName.c_str(), ELL_ERROR);
			return;
		}

		skipSpacesAndLineEnd();
	}

	bool CMD5MeshFileLoader::parseSection(Section &sec)
	{
		sec.iLineNumber = m_lineNumber;

		c8 *sz = m_buffer;
		while (!core::isSpaceOrNewLine(*m_buffer)) ++m_buffer;
		sec.mName = std::move(core::stringc(sz, m_buffer - sz));
		core::skipSpace(&m_buffer);

		while (true)
		{
			//{}
			if ('{' == *m_buffer)
			{
				++m_buffer;
				while (true)
				{
					//结束解析
					if (!skipSpacesAndLineEnd())
					{
						return false;  //到了最后一个section
					}
					//一个section结束
					if ('}' == *m_buffer)
					{
						++m_buffer;
						break;
					}

					sec.mElements.push_back(Element());
					auto &ele = sec.mElements.getLast();

					ele.iLineNumber = m_lineNumber;
					ele.szStart = m_buffer;

					//每行的\n改为\0结束
					while (!core::isLineEnd(*m_buffer)) ++m_buffer;
					if (*m_buffer) {
						++m_lineNumber;
						*m_buffer++ = '\0';
					}
				}
				break;
			}
			// 全局变量
			else if (!core::isSpaceOrNewLine(*m_buffer))
			{
				sz = m_buffer;
				while (!core::isSpaceOrNewLine(*m_buffer++));
				sec.mGlobalValue = std::move(core::stringc(sz, m_buffer - sz));
				continue;
			}
			break;
		}
		return skipSpacesAndLineEnd();
	}

	void CMD5MeshFileLoader::parseMesh()
	{
		os::Printer::log("parse md5 mesh file begin. name:", m_fileName.c_str(), ELL_DEBUG);

		//解析所有的section
		int secSize = m_sections.size();
		for (int i = 0; i < secSize; ++i)
		{
			auto &sec = m_sections[i];
			if (sec.mName == "numMeshes")
			{
				AnimatedMesh->getMeshBuffers().reallocate(core::strtoul10(sec.mGlobalValue.c_str()));
			}
			else if (sec.mName == "numJoints")
			{
				AnimatedMesh->getAllJoints().reallocate(core::strtoul10(sec.mGlobalValue.c_str()));
			}
			else if (sec.mName == "joints")
			{
				int eleSize = sec.mElements.size();
				for (int j = 0; j < eleSize; ++j)
				{
					const c8 *sz = sec.mElements[j].szStart;
					core::stringc jointName;
					if (!parseString(&sz, sec.mElements[j].iLineNumber, jointName)) continue;

					core::skipSpace(&sz);
					int parentIndex = core::strtoul10(sz, &sz);
					auto *parentJoint = parentIndex < 0 ? NULL : AnimatedMesh->getAllJoints()[parentIndex];
					auto *joint = AnimatedMesh->addJoint(parentJoint);

					joint->Name = jointName;
					parseVec(&sz, sec.mElements[j].iLineNumber, joint->Animatedposition);
					core::vector3df temp;
					parseVec(&sz, sec.mElements[j].iLineNumber, temp);
					convertVecToQuat(temp, joint->Animatedrotation);

					//build local matrix
					core::matrix4 positionMatrix;
					positionMatrix.setTranslation(joint->Animatedposition);
					core::matrix4 rotationMatrix;
					joint->Animatedrotation.getMatrix_transposed(rotationMatrix);

					joint->LocalMatrix = positionMatrix * rotationMatrix;
					if (parentJoint)
						joint->GlobalMatrix = parentJoint->GlobalMatrix * joint->LocalMatrix;
					else
						joint->GlobalMatrix = joint->LocalMatrix;
				}
			}
			else if (sec.mName == "mesh")
			{
				int textureLayer = 0;
				auto *mesh = AnimatedMesh->addMeshBuffer();
				int eleSize = sec.mElements.size();
				for (int j = 0; j < eleSize; ++j)
				{
					const c8 *sz = sec.mElements[j].szStart;

					//shader
					if (core::tokenMatch(&sz, "shader", 6))
					{
						core::skipSpace(&sz);
						core::stringc shaderName;
						if (!parseString(&sz, sec.mElements[j].iLineNumber, shaderName)) continue;
						mesh->Material.setTexture(textureLayer, getMeshTextureLoader() ? getMeshTextureLoader()->getTexture(shaderName) : NULL);
						++textureLayer;
						if (textureLayer == 2)
							mesh->Material.MaterialType = video::EMT_LIGHTMAP;
					}
					//verts
					else if (core::tokenMatch(&sz, "numverts", 8))
					{
						core::skipSpace(&sz);
						mesh->Vertices_Standard.reallocate(core::strtoul10(sz, &sz));
					}
					else if (core::tokenMatch(&sz, "numtris", 7))
					{
						core::skipSpace(&sz);
						mesh->Indices.reallocate(core::strtoul10(sz, &sz) * 3);
					}
					else if (core::tokenMatch(&sz, "numweights", 10))
					{
						//TODO
						core::skipSpace(&sz);
						

					}
					else if(core::tokenMatch(&sz, ""))
				}
			}
		}
	}

	void CMD5MeshFileLoader::_parseShader(const c8 * bufBegin, const c8 * bufEnd)
	{
		core::skipSpace(&bufBegin);

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

	bool CMD5MeshFileLoader::skipLine()
	{
		++m_lineNumber;
		return core::skipLine(&m_buffer);
	}


	bool CMD5MeshFileLoader::parseString(const c8 ** buf, int lineNumber, core::stringc &name)
	{
		bool bQuta = **buf == '\"';
		const c8 *szStart = *buf;
		while (!core::isSpaceOrNewLine(**buf)) ++(*buf);
		const c8 *szEnd = *buf;
		if (bQuta)
		{
			++szStart;
			if ('\"' != *(szEnd - 1))
			{
				os::Printer::log("Expected closing quotation marks in string. line:", core::stringc(lineNumber).c_str(), ELL_DEBUG);
				return false;
			}
		}

		name = std::move(core::stringc(szStart, szEnd - szStart));
		return name != "";
	}

	bool CMD5MeshFileLoader::parseVec(const c8 **buf, int lineNumber, core::vector3df & vec)
	{
		core::skipSpace(buf);
		if ('(' != **buf)
		{
			os::Printer::log("Unexpected token : (was expected ", core::stringc(lineNumber).c_str(), ELL_ERROR);
			++(*buf);
			return false;
		}

		*buf = core::fast_atof_move(*buf, vec.X);
		core::skipSpace(buf);

		*buf = core::fast_atof_move(*buf, vec.Y);
		core::skipSpace(buf);

		*buf = core::fast_atof_move(*buf, vec.Z);
		core::skipSpace(buf);

		if (')' != **buf)
		{
			os::Printer::log("Unexpected token : )was expected ", core::stringc(lineNumber).c_str(), ELL_ERROR);
			++(*buf);
			return false;
		}
	}

	void CMD5MeshFileLoader::convertVecToQuat(core::vector3df & vec, core::quaternion &quat)
	{
		quat.X = vec.X;
		quat.Y = vec.Y;
		quat.Z = vec.Z;

		const float t = 1.0f - (vec.X*vec.X) - (vec.Y*vec.Y) - (vec.Z*vec.Z);

		if (t < 0.0f)
			quat.W = 0.0f;
		else quat.W = sqrt(t);
	}

	bool CMD5MeshFileLoader::skipSpacesAndLineEnd()
	{
		auto p = m_buffer;
		bool bHad = false;
		bool running = true;
		while (running) {
			if (*p == '\r' || *p == '\n') {
				if (!bHad) {
					bHad = true;
					++m_lineNumber;
				}
			}
			else if (*p == '\t' || *p == ' ')bHad = false;
			else break;
			p++;
		}
		m_buffer = p;
		return *p != '\0';
	}

}
}

