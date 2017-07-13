#include "CMD5MeshFileLoader.h"

#include <math.h>

#include "CSkinnedMesh.h"
#include "CMeshTextureLoader.h"
#include "IReadFile.h"
#include "IFileSystem.h"
#include "ISceneManager.h"
#include "coreutil.h"
#include "fast_atof.h"
#include "os.h"

namespace irr
{
namespace scene
{

	CMD5MeshFileLoader::CMD5MeshFileLoader(scene::ISceneManager* smgr, io::IFileSystem* fs) : SceneManager(smgr), AnimatedMesh(NULL), FileSystem(fs)
	{
		#ifdef _DEBUG
			setDebugName("CMS3DMeshFileLoader");
		#endif

		TextureLoader = new CMeshTextureLoader(FileSystem, SceneManager->getVideoDriver());
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

		if (getMeshTextureLoader())
			getMeshTextureLoader()->setMeshFile(file);

		AnimatedMesh = new CSkinnedMesh();

		if (!loadMeshFile(file)) 
		{
			AnimatedMesh->drop();
			AnimatedMesh = 0;
			return NULL;
		}

		io::path path;
		io::path fname;
		io::path ext;
		core::splitFilename(file->getFileName(), &path, &fname, &ext);

		io::path animFile = path + fname + ".md5anim";
		if (FileSystem->existFile(animFile))
		{
			io::IReadFile* file = FileSystem->createAndOpenFile(animFile);
			if (!file)
			{
				os::Printer::log("Could not load mesh, because file could not be opened: ", filename, ELL_ERROR);
				return false;
			}
			else
			{
				if (!loadAnimFile(file))
				{
					AnimatedMesh->drop();
					AnimatedMesh = 0;
					return NULL;
				}
			}
		}

		AnimatedMesh->finalize();
		return AnimatedMesh;
	}

	bool CMD5MeshFileLoader::loadMeshFile(io::IReadFile *file) {
		
		if ()
		{
		}

		parseMesh();

		if (m_buffer)
		{
			delete m_buffer;
			m_buffer = 0;
		}

		m_meshs.clear();
		m_sections.clear();
		m_joints.clear();

		return true;
	}


	bool CMD5MeshFileLoader::loadAnimFile(io::IReadFile *file)
	{

	}


	bool CMD5MeshFileLoader::loadFileImp(io::IReadFile *file)
	{
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
		core::skipSpace((const char**)&m_buffer);
		if (!core::tokenMatch((const char**)&m_buffer, "MD5Version", 10))
		{
			os::Printer::log("Invalid MD5 file: MD5Version tag has not been found", m_fileName.c_str(), ELL_ERROR);
			return;
		}

		core::skipSpace((const char**)&m_buffer);
		m_header.version = core::strtoul10(m_buffer, (const char **)&m_buffer);
		if (m_header.version != 10)
		{
			os::Printer::log("MD5 version tag is unknown (10 is expected)", m_fileName.c_str(), ELL_ERROR);
			return;
		}

		//解析cmd
		skipLine();
		core::skipSpace((const char**)&m_buffer);
		if (!core::tokenMatch((const char**)&m_buffer, "commandline", 13))
		{
			os::Printer::log("Invalid MD5 file: commandline tag has not been found ", m_fileName.c_str(), ELL_ERROR);
			return;
		}

		core::skipSpace((const char**)&m_buffer);
		if (!parseString((const char**)&m_buffer, m_lineNumber, m_header.commondLineStr))
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
		core::skipSpace((const char**)&m_buffer);

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
				//AnimatedMesh->getMeshBuffers().reallocate(core::strtoul10(sec.mGlobalValue.c_str()));
				m_meshs.reallocate(core::strtoul10(sec.mGlobalValue.c_str()));
			}
			else if (sec.mName == "numJoints")
			{
				//AnimatedMesh->getAllJoints().reallocate(core::strtoul10(sec.mGlobalValue.c_str()));
				m_joints.reallocate(core::strtoul10(sec.mGlobalValue.c_str()));
			}
			else if (sec.mName == "joints")
			{
				int eleSize = sec.mElements.size();
				for (int j = 0; j < eleSize; ++j)
				{
					m_joints.push_back(MD5Bone());
					MD5Bone &bone = m_joints.getLast();

					c8 *sz = sec.mElements[j].szStart;
					if (!parseString((const char**)&sz, sec.mElements[j].iLineNumber, bone.mName)) continue;

					core::skipSpace((const char**)&sz);
					bone.mParentIndex = core::strtoul10(sz, (const char **)&sz);
					
					parseVec((const char**)&sz, sec.mElements[j].iLineNumber, bone.mPositionXYZ);
					parseVec((const char**)&sz, sec.mElements[j].iLineNumber, bone.mRotationQuat);
				}
			}
			else if (sec.mName == "mesh")
			{
				m_meshs.push_back(MD5Mesh());
				MD5Mesh &mesh = m_meshs.getLast();
				
				int eleSize = sec.mElements.size();
				for (int j = 0; j < eleSize; ++j)
				{
					const c8 *sz = sec.mElements[j].szStart;

					//shader
					if (core::tokenMatch((const char**)&sz, "shader", 6))
					{
						core::skipSpace((const char**)&sz);
						if (!parseString(&sz, sec.mElements[j].iLineNumber, mesh.mShader)) continue;
					}
					//verts
					else if (core::tokenMatch((const char**)&sz, "numverts", 8))
					{
						core::skipSpace((const char**)&sz);
						mesh.mVertices.reallocate(core::strtoul10(sz, (const char **)&sz));
					}
					else if (core::tokenMatch((const char**)&sz, "numtris", 7))
					{
						core::skipSpace((const char**)&sz);
						mesh.mFaces.reallocate(core::strtoul10(sz, (const char **)&sz));
					}
					else if (core::tokenMatch((const char**)&sz, "numweights", 10))
					{
						core::skipSpace((const char**)&sz);
						mesh.mWeights.reallocate(core::strtoul10(sz, &sz));

					}
					else if (core::tokenMatch((const char**)&sz, "vert", 4))
					{
						core::skipSpace((const char**)&sz);
						const unsigned int idx = core::strtoul10(sz, &sz);
						core::skipSpace((const char**)&sz);
						if (idx >= mesh.mVertices.size())
						{
							mesh.mVertices.reallocate(idx + 1);
						}

						MD5Vetex &vertex = mesh.mVertices[idx];
						if ('(' != *sz++)
						{
							os::Printer::log("Unexpected token: ( was expected", core::stringc(sec.mElements[j].iLineNumber), ELL_ERROR);
							return;
						}
						core::skipSpace((const char**)&sz);
						sz = core::fast_atof_move(sz, vertex.mUV.X);
						core::skipSpace((const char**)&sz);
						sz = core::fast_atof_move(sz, vertex.mUV.Y);
						core::skipSpace((const char**)&sz);
						if (')' != *sz++)
						{
							os::Printer::log("Unexpected token: ) was expected", core::stringc(sec.mElements[j].iLineNumber), ELL_ERROR);
							return;
						}
						core::skipSpace((const char**)&sz);
						vertex.mFirstWeight = core::strtoul10(sz, &sz);
						core::skipSpace((const char**)&sz);
						vertex.mNumWeights = core::strtoul10(sz, &sz);

					}
					else if (core::tokenMatch((const char**)&sz, "tri", 3))
					{
						core::skipSpace((const char**)&sz);
						const unsigned int idx = core::strtoul10(sz, &sz);
						if (idx >= mesh.mFaces.size())
						{
							mesh.mFaces.reallocate(idx+1);
						}
						mesh.mFaces[idx].mNumIndices = 3;
						core::skipSpace((const char**)&sz);
						mesh.mFaces[idx].mIndices.X = core::strtoul10(sz, &sz);
						core::skipSpace((const char**)&sz);
						mesh.mFaces[idx].mIndices.Y = core::strtoul10(sz, &sz);
						core::skipSpace((const char**)&sz);
						mesh.mFaces[idx].mIndices.Z = core::strtoul10(sz, &sz);

					}
					else if (core::tokenMatch((const char**)&sz, "weight", 6))
					{
						core::skipSpace((const char**)&sz);
						const unsigned int idx = core::strtoul10(sz, &sz);
						core::skipSpace((const char**)&sz);
						if (idx >= mesh.mWeights.size())
							mesh.mWeights.reallocate(idx + 1);

						MD5Weight &weight = mesh.mWeights[idx];
						weight.mBone = core::strtoul10(sz, &sz);
						core::skipSpace((const char**)&sz);
						sz = core::fast_atof_move((const char *)sz, weight.mWeight);
						parseVec(&sz, sec.mElements[j].iLineNumber, weight.mOffsetPosition);
					}
				}
			}
		}
		os::Printer::log("parse md5 mesh file end. name:", m_fileName.c_str(), ELL_DEBUG);

		//转化为CSkinnedMesh

		//先转化 骨骼
		for (u32 i = 0; i < m_joints.size(); ++i)
		{
			MD5Bone &bone = m_joints[i];
			auto parentJoint = bone.mParentIndex == -1 ? NULL : AnimatedMesh->getAllJoints()[bone.mParentIndex];
			auto *joint = AnimatedMesh->addJoint(parentJoint);
			joint->Animatedposition = bone.mPositionXYZ;
			convertVecToQuat(bone.mRotationQuat, bone.mRotationQuatConverted);
			joint->Animatedrotation = bone.mRotationQuatConverted;
			joint->Name = bone.mName;

			//计算矩阵
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

		//转换 mesh
		for (u32 i = 0; i < m_meshs.size(); ++i)
		{
			MD5Mesh &mesh = m_meshs[i];
			auto *aMesh = AnimatedMesh->addMeshBuffer();

			aMesh->Vertices_2TCoords.reallocate(mesh.mVertices.size());
			aMesh->Indices.reallocate(mesh.mVertices.size() * 3);
			for (u32 j = 0; j < mesh.mVertices.size(); ++j)
			{
				aMesh->Vertices_2TCoords[j].TCoords.X = mesh.mVertices[j].mUV.X;
				aMesh->Vertices_2TCoords[j].TCoords.Y = mesh.mVertices[j].mUV.Y;
				aMesh->VertexType = video::EVT_2TCOORDS;
				aMesh->Indices[3 * j] = mesh.mFaces[j].mIndices.X;
				aMesh->Indices[3 * j + 1] = mesh.mFaces[j].mIndices.Y;
				aMesh->Indices[3 * j + 2] = mesh.mFaces[j].mIndices.Z;

				for (u32 k = 0; k < mesh.mVertices[j].mNumWeights; ++k)
				{ 
					auto &weight = mesh.mWeights[mesh.mVertices[j].mFirstWeight + k];
					auto &joint = m_joints[weight.mBone];
					weight.mVertexID = j;
					const core::vector3df v = joint.mRotationQuatConverted * weight.mOffsetPosition;

					aMesh->Vertices_2TCoords[j].Pos += (joint.mPositionXYZ + v) * weight.mWeight;
				}
			}

			for (u32 j = 0; j < mesh.mWeights.size(); ++j)
			{
				const MD5Weight &weight = mesh.mWeights[j];

				auto *joint = AnimatedMesh->getAllJoints()[weight.mBone];
				joint->AttachedMeshes.push_back(AnimatedMesh->getMeshBufferCount() - 1);

				auto *aWeight = AnimatedMesh->addWeight(joint);
				aWeight->buffer_id = AnimatedMesh->getMeshBufferCount() - 1;
				aWeight->strength = weight.mWeight;
				aWeight->vertex_id = weight.mVertexID;
			}

			int textureLayer = 0;
			aMesh->Material.setTexture(textureLayer, getMeshTextureLoader() ? getMeshTextureLoader()->getTexture(mesh.mShader) : NULL);
		}
	}


	void CMD5MeshFileLoader::parseAnim()
	{

	}

	bool CMD5MeshFileLoader::skipLine()
	{
		++m_lineNumber;
		return core::skipLine((const char**)&m_buffer);
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

		return true;
	}

	void CMD5MeshFileLoader::convertVecToQuat(const core::vector3df & vec, core::quaternion &quat)
	{
		quat.X = vec.X;
		quat.Y = vec.Y;
		quat.Z = vec.Z;

		const float t = 1.0f - (vec.X*vec.X) - (vec.Y*vec.Y) - (vec.Z*vec.Z);

		if (t < 0.0f)
			quat.W = 0.0f;
		else quat.W = float(sqrt(t)); 

		quat.W *= -1.f;
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

