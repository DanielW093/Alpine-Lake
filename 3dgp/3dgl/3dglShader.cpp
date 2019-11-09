#include "../include/glee.h"
#include "../include/3dglShader.h"

#include <fstream>
#include <vector>

using namespace std;
using namespace _3dgl;

/////////////////////////////////////////////////////////////////////////////////////////////////
// C3dglShader

bool C3dglShader::Create(GLenum type)
{
	m_type = type;
	m_id = glCreateShader(m_type);
	if (m_id == 0) return logError("Shader creation error. Wrong type of shader.");
	return logSuccess("created successfully.");
}

bool C3dglShader::Load(std::string source)
{
	if (m_id == 0) return logError("Shader creation error. Wrong type of shader.");
	m_source = source;
	if (m_source.empty()) return false;
	const GLchar *pSource = static_cast<const GLchar*>(m_source.c_str());
	glShaderSource(m_id, 1, &pSource, NULL);
	return logSuccess("source code loaded.");		// always successful
}

bool C3dglShader::LoadFromFile(std::string fname)
{
	m_fname = fname;
	ifstream file(m_fname.c_str());
	string source(istreambuf_iterator<char>(file), (istreambuf_iterator<char>()));
	return Load(source);
}

bool C3dglShader::Compile()
{
	if (m_id == 0) return logError("Shader creation error. Wrong type of shader.");

	// compile
	glCompileShader(m_id);

	// check status
	GLint result = 0;
	glGetShaderiv(m_id, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		// collect info log
		GLint infoLen;
		glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &infoLen);
		vector<char> log(infoLen);
		glGetShaderInfoLog(m_id, log.size(), &infoLen, &log[0]);
		return logError(string(log.begin(), log.end()));
	}
	return logSuccess("compiled successfully.");
}

std::string C3dglShader::getName()
{
	switch (m_type)
	{
	case GL_VERTEX_SHADER: return "Vertex Shader";
	case GL_FRAGMENT_SHADER: return "Fragment Shader";
	//case GL_COMPUTE_SHADER: return "Compute Shader";
	//case GL_TESS_CONTROL_SHADER: return "Tesselation Control Shader";
	//case GL_TESS_EVALUATION_SHADER: return "Tesselation Evaluation Shader";
	//case GL_GEOMETRY_SHADER: return "Geometry Shader";
	default: return "Shader";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// C3dglProgram

C3dglProgram *C3dglProgram::c_pCurrentProgram = NULL;

C3dglProgram::C3dglProgram() : C3dglObject()
{
	m_id = 0;
	memset(m_stdAttr, -1, sizeof(m_stdAttr));
	memset(m_stdUni, -1, sizeof(m_stdUni));
}

bool C3dglProgram::Create()
{
	m_id = glCreateProgram();
	if (m_id == 0) return logError("creation error.");
	return logSuccess("created successfully.");
}

bool C3dglProgram::Attach(C3dglShader &shader)
{
	if (m_id == 0) return logError("not created.");
	if (shader.getId() == 0) return logError("cannot attach shader: Shader not created.");

	glAttachShader(m_id, shader.getId());
	return logSuccess("has successfully attached a " + shader.getName());
}

bool C3dglProgram::Link(std::string std_attrib_names, std::string std_uni_names)
{
	if (m_id == 0) return logError("not created.");

	// link
	glLinkProgram(m_id);

	// check status
	GLint result = 0;
	glGetProgramiv(m_id, GL_LINK_STATUS, &result);
	if (!result)
	{
		// collect info log
		GLint infoLen;
		glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLen);
		vector<char> log(infoLen);
		glGetProgramInfoLog(m_id, log.size(), &infoLen, &log[0]);
		return logError("linking error: " + string(log.begin(), log.end()));
	}

	// Collect Standard Attribute Locations
	string STD_ATTRIB_NAMES[] = {
		"a_vertex|a_Vertex|aVertex|avertex|vertex|Vertex",
		"a_normal|a_Normal|aNormal|anormal|normal|Normal",
		"a_texcoord|a_TexCoord|aTexCoord|atexcoord|texcoord|TexCoord",
		"a_tangent|a_Tangent|aTangent|atangent|tangent|Tangent",
		"a_bitangent|a_Bitangent|aBitangent|abitangent|bitangent|Bitangent|a_biTangent|a_BiTangent|aBiTangent|abiTangent|biTangent|BiTangent",
		"a_color|a_Color|aColor|acolor|color|Color",
		"a_boneid|a_Boneid|aBoneid|aboneid|boneid|Boneid|a_boneId|a_BoneId|aBoneId|aboneId|boneId|BoneId|"
		"a_boneids|a_Boneids|aBoneids|aboneids|boneids|Boneids|a_boneIds|a_BoneIds|aBoneIds|aboneIds|boneIds|BoneIds",
		"a_boneweight|a_Boneweight|aBoneweight|aboneweight|boneweight|Boneweight|a_boneWeight|a_BoneWeight|aBoneWeight|aboneWeight|boneWeight|BoneWeight|a_weight|aweight|weight|a_Weight|aWeight|Weight|"
		"a_boneweights|a_Boneweights|aBoneweights|aboneweights|boneweights|Boneweights|a_boneWeights|a_BoneWeights|aBoneWeights|aboneWeights|boneWeights|BoneWeights|a_weights|aweights|weights|a_Weights|aWeights|Weights",
	};
	int astart = 0, aend = 0;
	std_attrib_names += ";";
	for (GLuint i = 0; i < ATTR_LAST; i++)
	{
		string str = "";
		aend = std_attrib_names.find(";", astart);
		if (aend != string::npos)
		{
			str = std_attrib_names.substr(astart, aend - astart);
			astart = aend + 1;
		}
		if (str.empty()) str = STD_ATTRIB_NAMES[i];
		str += "|";

		int nstart = 0, nend = 0;
		while ((nend = str.find("|", nstart)) != string::npos)
		{
			string name = str.substr(nstart, nend - nstart);
			nstart = nend + 1;
			if (name.empty()) continue;
			
			m_stdAttr[i] = glGetAttribLocation(m_id, name.c_str());
			if (m_stdAttr[i] != (GLuint)-1)
			{
				logSuccess("attribute location found: " + name + " = " + to_string(m_stdAttr[i]));
				break;
			}
		}
	}
	
	// Collect Standard Attribute Locations
	string STD_UNI_NAMES[] = {
		"modelview_matrix|modelView_matrix|ModelView_matrix|Modelview_matrix|modelview_Matrix|modelView_Matrix|ModelView_Matrix|Modelview_Matrix|"
		"matrix_modelview|matrix_modelView|matrix_ModelView|matrix_Modelview|Matrix_modelview|Matrix_modelView|Matrix_ModelView|Matrix_Modelview|" 
		"modelviewmatrix|modelViewmatrix|ModelViewmatrix|Modelviewmatrix|modelviewMatrix|modelViewMatrix|ModelViewMatrix|ModelviewMatrix|" 
		"matrixmodelview|matrixmodelView|matrixModelView|matrixModelview|Matrixmodelview|MatrixmodelView|MatrixModelView|MatrixModelview|",
		"mat_ambient|material_ambient|mat_Ambient|material_Ambient|matambient|materialambient|matAmbient|materialAmbient",
		"mat_diffuse|material_diffuse|mat_Diffuse|material_Diffuse|matdiffuse|materialdiffuse|matDiffuse|materialDiffuse",
		"mat_specular|material_specular|mat_Specular|material_Specular|matspecular|materialspecular|matSpecular|materialSpecular",
		"mat_emissive|material_emissive|mat_Emissive|material_Emissive|matemissive|materialemissive|matEmissive|materialEmissive",
		"shininess|Shininess|mat_shininess|material_shininess|mat_Shininess|material_Shininess|matshininess|materialshininess|matShininess|materialShininess"
	};
	int lstart = 0, lend = 0;
	std_uni_names += ";";
	for (GLuint i = 0; i < UNI_LAST; i++)
	{
		string str = "";
		lend = std_uni_names.find(";", lstart);
		if (lend != string::npos)
		{
			str = std_uni_names.substr(lstart, lend - lstart);
			lstart = lend + 1;
		}
		if (str.empty()) str = STD_UNI_NAMES[i];
		str += "|";

		int nstart = 0, nend = 0;
		while ((nend = str.find("|", nstart)) != string::npos)
		{
			string name = str.substr(nstart, nend - nstart);
			nstart = nend + 1;
			if (name.empty()) continue;

			m_stdUni[i] = glGetUniformLocation(m_id, name.c_str());
			if (m_stdUni[i] != (GLuint)-1)
			{
				logSuccess("uniform location found: " + name + " = " + to_string(m_stdUni[i]));
				break;
			}
		}
	}

	return logSuccess("linked successfully.");
}

bool C3dglProgram::Use(bool bValidate)
{
	if (m_id == 0) return logError("not created.");
	glUseProgram(m_id);

	c_pCurrentProgram = this;

	if (!bValidate) return true;

	glValidateProgram(m_id);

	// collect info log
	GLint infoLen;
	glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLen);
	vector<char> log(infoLen);
	glGetProgramInfoLog(m_id, log.size(), &infoLen, &log[0]);
	return logSuccess("verification result: " + (log.size() <= 1 ? "OK" : string(log.begin(), log.end())));
}

GLuint C3dglProgram::GetAttribLocation(std::string idAttrib)
{
	auto i = m_attribs.find(idAttrib);
	if (i == m_attribs.end())
	{
		GLuint nAttrib = glGetAttribLocation(m_id, idAttrib.c_str());
		m_attribs[idAttrib] = nAttrib;
		return nAttrib;
	}
	else
		return i->second;
}

GLuint C3dglProgram::GetUniformLocation(std::string idUniform)
{
	auto i = m_uniforms.find(idUniform);
	if (i == m_uniforms.end())
	{
		GLuint nUniform = glGetUniformLocation(m_id, idUniform.c_str());
		m_uniforms[idUniform] = nUniform;
		if (nUniform == (GLuint)-1) logWarning("uniform location not found: " + idUniform);
		return nUniform;
	}
	else
		return i->second;
}
