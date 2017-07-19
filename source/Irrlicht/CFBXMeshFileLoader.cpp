#include "CFBXMeshFileLoader.h"

#include <math.h>
#include <assert.h>

#include "CSkinnedMesh.h"
#include "CMeshTextureLoader.h"
#include "IReadFile.h"
#include "IFileSystem.h"
#include "ISceneManager.h"
#include "coreutil.h"
#include "fast_atof.h"
#include "os.h"

#ifdef _WIN32
	#ifdef _DEBUG
		#pragma comment(lib, "FBX/2016.1.2/lib/vs2015/x86/debug/libfbxsdk.lib")
	#else
		#pragma comment(lib, "FBX/2016.1.2/lib/vs2015/x86/release/libfbxsdk.lib")
	#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib, "FBX/2016.1.2/lib/vs2015/x64/debug/libfbxsdk.lib")
	#else
		#pragma comment(lib, "FBX/2016.1.2/lib/vs2015/x64/release/libfbxsdk.lib")
	#endif
#endif

using namespace fbxsdk;

namespace irr
{
namespace scene
{
	CFBXMeshFileLoader::CFBXMeshFileLoader(scene::ISceneManager * smgr, io::IFileSystem * fs)
	{
		#ifdef _DEBUG
		setDebugName("CFBXMeshFileLoader");
		#endif

		m_fbxManager = FbxManager::Create();
		if (!m_fbxManager)
		{
			os::Printer::log("Error: Unable to create FBX Manager!", ELL_ERROR);
			assert(true);
		}
		os::Printer::log("Autodesk FBX SDK version ", m_fbxManager->GetVersion(), ELL_DEBUG);

		FbxIOSettings *ios = FbxIOSettings::Create(m_fbxManager, IOSROOT);
		m_fbxManager->SetIOSettings(ios);

		m_fbxScene = FbxScene::Create(m_fbxManager, "");
		if (!m_fbxScene)
		{
			os::Printer::log("Error: Unable to create FBX Scene!", ELL_ERROR);
			assert(true);
		}

		TextureLoader = new CMeshTextureLoader(fs, smgr->getVideoDriver());
	}

	CFBXMeshFileLoader::~CFBXMeshFileLoader()
	{
		if (m_fbxManager)
		{
			m_fbxManager->Destroy();
		}

		if (m_fbxScene)
		{
			m_fbxScene->Destroy(true);
		}
	}

	bool CFBXMeshFileLoader::isALoadableFileExtension(const io::path& filename) const
	{
		return core::hasFileExtension(filename, "fbx");
	}


	IAnimatedMesh * CFBXMeshFileLoader::createMesh(io::IReadFile *file)
	{
		FbxImporter *importer = FbxImporter::Create(m_fbxManager, "");
		bool bRes = importer->Initialize(file->getFileName().c_str(), -1, m_fbxManager->GetIOSettings());
		if (!bRes)
		{
			os::Printer::log("Error: can't load file, name:", file->getFileName().c_str(), ELL_ERROR);
			return NULL;
		}

		bRes = importer->Import(m_fbxScene);
		if (!bRes)
		{
			os::Printer::log("Could not parse FBX file, name:", file->getFileName().c_str(), ELL_ERROR);
			return NULL;
		}

		int lFileMajor, lFileMinor, lFileRevision;
		importer->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);
		core::stringc temp(file->getFileName());
		temp.append(" is ");
		temp.append(lFileMajor);
		temp.append(".");
		temp.append(lFileMinor);
		temp.append(".");
		temp.append(lFileRevision);

		os::Printer::log("FBX file format version for file, name:", temp.c_str(), ELL_DEBUG);

		//m_fbxTransform.in
	}

	void CFBXMeshFileLoader::CollectSceneNodes(FbxNode * pNode)
	{
        if (!pNode)
        {
            return;
        }

		auto *attribute = pNode->GetNodeAttribute();
		if (attribute)
		{
			switch (attribute->GetAttributeType())
			{
			case FbxNodeAttribute::eMesh:
                m_fbxMeshNodes.insert(pNode);
				break;
			case FbxNodeAttribute::eLight:
                m_fbxLightNodes.insert(pNode);
				break;
			case FbxNodeAttribute::eSkeleton:
                m_fbxBoneNodes[pNode] = false;
				break;
			case FbxNodeAttribute::eCamera:
                m_fbxCameraNodes.insert(pNode);
				break;
			default:
				break;
			}
		}

		for (u32 i = 0; i < pNode->GetChildCount(); ++i)
		{
            CollectSceneNodes(pNode->GetChild(i));
		}
	}

    void CFBXMeshFileLoader::ComputeSkeletonBindPose(FbxNode * pNode)
    {
        if (pNode || pNode->GetMesh())
        {
            return;
        }

        FbxMesh *pMesh = pNode->GetMesh();
        for (int i = 0; i < pMesh->GetDeformerCount(); ++i)
        {
            FbxDeformer *deformer = pMesh->GetDeformer(i);
            if (!deformer)
            {
                continue;
            }

            if (deformer->GetDeformerType() == FbxDeformer::eSkin)
            {
                FbxSkin *skin = FbxCast<FbxSkin>(deformer);
                if (!skin)
                {
                    continue;
                }

                for (int j = 0; j < skin->GetClusterCount(); ++j)
                {
                    FbxCluster *cluster = skin->GetCluster(j);
                    if (!cluster)
                    {
                        continue;
                    }

                    //TODO
                }
            }
        }
    }

	void CFBXMeshFileLoader::processMesh(FbxNode * pNode)
	{
		FbxMesh *pMesh = pNode->GetMesh();
		int count = pMesh->GetPolygonCount();
		for (int i = 0; i < count; ++i)
		{
			for (int j = 0; j < pMesh->GetPolygonVertexCount(); ++j)
			{
				int vIndex = pMesh->GetPolygonVertex(i, j);
				readVertex(pMesh, vIndex);
			}
		}
	}

	void CFBXMeshFileLoader::processSkeleton(FbxNode * pNode)
	{
	}

	void CFBXMeshFileLoader::processLight(FbxNode * pNode)
	{
	}

	void CFBXMeshFileLoader::processCamera()
	{
	}

	void CFBXMeshFileLoader::loadMaterial()
	{
		for (int i = 0; i < m_fbxScene->GetMaterialCount(); ++i)
		{
			auto *pMaterial = m_fbxScene->GetMaterial(i);
			MaterialData matData;
			matData.mName = pMaterial->GetName();

			f32 fTransparency = 1.0f;

			FbxProperty emissiveProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sEmissive);
			FbxProperty diffuseProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
			FbxProperty specularProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sSpecular);
			FbxProperty ambientProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sAmbient);
			FbxProperty speularFactorProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sSpecularFactor);
			FbxProperty shinessProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sShininess);
			FbxProperty normalMapProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap);
			FbxProperty tansparentFactorProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);

			FbxSurfaceLambert *lambert = FbxCast<FbxSurfaceLambert>(pMaterial);
			if (lambert)
			{
				matData.mAmbient = readMaterialColor(lambert->Ambient, lambert->AmbientFactor);
				matData.mDiffuse = readMaterialColor(lambert->Diffuse, lambert->DiffuseFactor);
				matData.mEmissive = readMaterialColor(lambert->Emissive, lambert->EmissiveFactor);
				if (lambert->TransparencyFactor.IsValid())
				{
					fTransparency = lambert->TransparencyFactor.Get();
				}
			}

			FbxSurfacePhong *phong = FbxCast<FbxSurfacePhong>(pMaterial);
			if (phong)
			{
				if (phong->Specular.IsValid())
				{
					//matData.mSpecular = readMaterialColor(phong->Specular, phong->SpecularFactor);
					matData.mSpecular.X = phong->Specular[0];
					matData.mSpecular.Y = phong->Specular[1];
					matData.mSpecular.Z = phong->Specular[2];
				}

				if (phong->Shininess.IsValid())
				{
					matData.mPower = phong->Shininess.Get();
				}
			}

			//Œ∆¿Ì
			for (int textureLayerIndex = 0; textureLayerIndex < FbxLayerElement::sTypeTextureCount; ++textureLayerIndex)
			{
				FbxProperty textureProperty = pMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[textureLayerIndex]);
				if (textureProperty.IsValid())
				{
					int textureCount = textureProperty.GetSrcObjectCount(FbxTexture::ClassId);
					for (int tex = 0; tex < textureCount; ++tex)
					{
						auto *pTex = FbxCast<FbxTexture>(textureProperty.GetSrcObject(tex));
						if (pTex)
						{
							FbxFileTexture *pFileTex = FbxCast<FbxFileTexture>(pTex);
							core::stringc filepath = pFileTex->GetFileName();
							matData.mTextures[FbxLayerElement::sTextureChannelNames[textureLayerIndex]] = filepath;
						}
					}
				}
			}

			m_materials.push_back(std::move(matData));
		}
	}

	core::vector3df CFBXMeshFileLoader::readMaterialColor(fbxsdk::FbxPropertyT<FbxDouble3> colorProperty, fbxsdk::FbxPropertyT<FbxDouble> colorFactorProperty)
	{
		core::vector3df color;
		if (colorProperty.IsValid())
		{
			color.X = colorProperty.Get()[0];
			color.Y = colorProperty.Get()[1];
			color.Z = colorProperty.Get()[2];
		}

		if (colorFactorProperty.IsValid())
		{
			color *= colorFactorProperty.Get();
		}

		return color;
	}

	void CFBXMeshFileLoader::loadMaterialAttribute(FbxSurfaceMaterial * pMaterial)
	{
		if (pMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
		{
			//TODO load attribute
		}

		if (pMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
		{
			//TODO load attribute
		}
	}

	void CFBXMeshFileLoader::loadMaterialTexture(FbxSurfaceMaterial * pMaterial)
	{
		FbxProperty pProperty;
		for (int textureLayerIndex = 0; textureLayerIndex < FbxLayerElement::eTypeCount; ++textureLayerIndex)
		{
			pProperty = pMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[textureLayerIndex]);
			if (pProperty.IsValid())
			{
				int textureCount = pProperty.GetSrcObjectCount(FbxTexture::ClassId);
				for (int j = 0; j < textureCount; ++j)
				{
					FbxTexture *pTexture = FbxCast<FbxTexture>(pProperty.GetSrcObject(FbxTexture::ClassId, j));
					if (pTexture)
					{
						//TODO
					}
				}
			}
		}
	}

	void CFBXMeshFileLoader::readVertex(FbxMesh * pMesh, int vIndex)
	{
		auto pCtrlPoint = pMesh->GetControlPoints();

		//TODOÃÓ–¥∂•µ„
	}

	void CFBXMeshFileLoader::readColor(FbxMesh * pMesh, int vIndex, int cIndex)
	{
		if (pMesh->GetElementVertexColorCount() < 1)
		{
			return;
		}

		auto *pVertexColor = pMesh->GetElementVertexColor(0);
		switch (pVertexColor->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			{
				switch (pVertexColor->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
					//TODO
					break;
				case FbxGeometryElement::eIndexToDirect:
					//TODO
					break;
				default:
					break;
				}
			}
			break;
		case FbxGeometryElement::eByPolygonVertex:
			{
				switch (pVertexColor->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
					//TODO
					break;
				case FbxGeometryElement::eIndexToDirect:
					//TODO
					break;
				default:
					break;
				}
			}
			break;
		default:
			break;
		}
	}

	void CFBXMeshFileLoader::readUV(FbxMesh * pMesh, int vIndex, int uvIndex, int uvLayer)
	{
		if (uvLayer >= pMesh->GetElementUVCount())
		{
			return;
		}

		auto *pVertexUV = pMesh->GetElementUV(uvLayer);
		switch (pVertexUV->GetMappingMode())
		{
			case FbxGeometryElement::eByControlPoint:
			{
				switch (pVertexUV->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
					//TODO
					break;
				case FbxGeometryElement::eIndexToDirect:
					//TODO
					break;
				default:
					break;
				}
			}
			break;
		case FbxGeometryElement::eByPolygonVertex:
			{
				switch (pVertexUV->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
					//TODO
					break;
				case FbxGeometryElement::eIndexToDirect:
					//TODO
					break;
				default:
					break;
				}
			}
			break;
		default:
			break;
		}
	}

	void CFBXMeshFileLoader::readNormal(fbxsdk::FbxMesh * pMesh, int vIndex, int cIndex)
	{
		if (pMesh->GetElementNormalCount() < 1)
		{
			return;
		}

		auto *pNormal = pMesh->GetElementNormal(0);
		switch (pNormal->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			{
				switch (pNormal->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
					//TODO
					break;
				case FbxGeometryElement::eIndexToDirect:
					//TODO
					break;
				default:
					break;
				}
			}
			break;
		case FbxGeometryElement::eByPolygonVertex:
			{
				switch (pNormal->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
					//TODO
					break;
				case FbxGeometryElement::eIndexToDirect:
					//TODO
					break;
				default:
					break;
				}
			}
			break;
		default:
			break;
		}
	}

	void CFBXMeshFileLoader::readTangent(FbxMesh * pMesh, int vIndex, int cIndex)
	{
		if (pMesh->GetElementTangentCount() < 1)
		{
			return;
		}

		auto *pTangent = pMesh->GetElementTangent(0);
		switch (pTangent->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			{
				switch (pTangent->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
					//TODO
					break;
				case FbxGeometryElement::eIndexToDirect:
					//TODO
					break;
				default:
					break;
				}
			}
			break;
		case FbxGeometryElement::eByPolygonVertex:
			{
				switch (pTangent->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
					//TODO
					break;
				case FbxGeometryElement::eIndexToDirect:
					//TODO
					break;
				default:
					break;
				}
			}
			break;
		default:
			break;
		}
	}
}
}

