#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "pch.h"
#include "Models.h"
#include "GUI.h"

extern spdlog::logger logger;


void initializeGLEW(void)
{
    GLenum err{ glewInit() };
    if (GLEW_OK != err) {
        logger.error("========== [GLEW]: Initialization failed =====================================");
        std::string text{ "\tError:" };
        text += (const char*)glewGetErrorString(err);
        logger.error(text);
    }
    std::string text{ "========== [GLEW]: Using GLEW " };
    text += (const char*)glewGetString(GLEW_VERSION);
    text += " =========================================";
    logger.info(text);

    // glewIsSupported supported from version 1.3
    if (GLEW_VERSION_1_3) {
        std::string versionName{ "GL_VERSION_4_5" };
        std::string extensionList[] = {
            "GL_ARB_separate_shader_objects",
            "GL_ARB_shader_image_load_store",
            "GL_ARB_texture_storage",
            "GL_ARB_vertex_attrib_binding",
            "GL_ARB_vertex_attrib_64bit",
            "GL_KHR_debug",
            "GL_NV_shader_buffer_load"
        };
        for (auto& ext : extensionList) {
            if (!glewIsSupported((versionName + " " + ext).c_str())) {
                text.clear();

                text = "========== [GLEW]: For " + versionName + " extension " + ext + " isn't supported ";
                logger.warn(text);
            }
        }
    }
    else {
        logger.warn("========== [GLEW]: OpenGL's extensions support haven't been verified! ============================");
    }
}



GLuint wrapMode(AkWrapMode& wrap) {
    GLuint wrap_m = GL_REPEAT;
    switch (wrap) {
    case AK_WRAP_MODE_WRAP:         wrap_m = GL_REPEAT; break;
    case AK_WRAP_MODE_MIRROR:       wrap_m = GL_MIRRORED_REPEAT; break;
    case AK_WRAP_MODE_CLAMP:        wrap_m = GL_CLAMP_TO_EDGE; break;
    case AK_WRAP_MODE_BORDER:       wrap_m = GL_CLAMP_TO_BORDER; break;
    case AK_WRAP_MODE_MIRROR_ONCE:  wrap_m = GL_MIRROR_CLAMP_TO_EDGE; break;
    }
    return wrap_m;
}


void setUpColor(
    AkColorDesc* colordesc,
    AkMeshPrimitive* prim,
    Drawable& primitive,
    enum TextureType type,
    GUI& panelConfig)
{
    GLuint* sampler = &primitive.samplers[type];
    GLuint* texture = &primitive.textures[type];
    GLuint* texturesType = &primitive.texturesType[type];
    glm::vec4* colors = &primitive.colors[type];

    if (colordesc) {
        if (colordesc->texture) {
            AkTextureRef* tex{ colordesc->texture };
            if (tex->texture) {
                AkSampler* samp{ tex->texture->sampler };
                if (!samp) return;

                AkTypeId type{ tex->texture->type };

                GLuint texture_type;
                GLuint minfilter, magfilter, mipfilter;
                GLuint wrap_t, wrap_s, wrap_p;
                switch (type) {
                case AKT_SAMPLER1D:     texture_type = GL_TEXTURE_1D; break;
                case AKT_SAMPLER2D:     texture_type = GL_TEXTURE_2D; break;
                case AKT_SAMPLER3D:     texture_type = GL_TEXTURE_3D; break;
                case AKT_SAMPLER_CUBE:  texture_type = GL_TEXTURE_CUBE_MAP; break;
                case AKT_SAMPLER_RECT:  texture_type = GL_TEXTURE_RECTANGLE; break;
                case AKT_SAMPLER_DEPTH: texture_type = GL_TEXTURE_2D; break;
                    break;
                }
                wrap_t = wrapMode(samp->wrapT);
                wrap_s = wrapMode(samp->wrapS);
                wrap_p = wrapMode(samp->wrapP);

                switch (samp->minfilter) {
                case AK_MINFILTER_LINEAR:       minfilter = GL_LINEAR; break;
                case AK_MINFILTER_NEAREST:      minfilter = GL_NEAREST; break;
                case AK_LINEAR_MIPMAP_NEAREST:  minfilter = GL_LINEAR_MIPMAP_NEAREST; break;
                case AK_LINEAR_MIPMAP_LINEAR:   minfilter = GL_LINEAR_MIPMAP_LINEAR; break;
                case AK_NEAREST_MIPMAP_NEAREST: minfilter = GL_NEAREST_MIPMAP_NEAREST; break;
                case AK_NEAREST_MIPMAP_LINEAR:  minfilter = GL_NEAREST_MIPMAP_LINEAR; break;
                }

                switch (samp->magfilter) {
                case AK_MAGFILTER_LINEAR:   magfilter = GL_LINEAR; break;
                case AK_MAGFILTER_NEAREST:  magfilter = GL_NEAREST; break;
                }

                switch (samp->mipfilter) {
                case AK_MIPFILTER_LINEAR:    mipfilter = GL_LINEAR; break;
                case AK_MIPFILTER_NEAREST:   mipfilter = GL_NEAREST; break;
                case AK_MIPFILTER_NONE:      mipfilter = GL_NONE; break;
                }

                glCreateSamplers(1, sampler);
                glSamplerParameteri(*sampler, GL_TEXTURE_WRAP_S, wrap_s);
                glSamplerParameteri(*sampler, GL_TEXTURE_WRAP_T, wrap_t);
                glSamplerParameteri(*sampler, GL_TEXTURE_WRAP_R, wrap_p);
                glSamplerParameteri(*sampler, GL_TEXTURE_MIN_FILTER, minfilter);
                glSamplerParameteri(*sampler, GL_TEXTURE_MAG_FILTER, magfilter);


                AkInput* tex_coord = ak_meshInputGet(prim, tex->coordInputName, tex->slot); //
                auto components{ 0 };
                auto width{ 0 };
                auto height{ 0 };
                char path[128] = { '\0' };
                memcpy_s(path, 128, panelConfig.getModelPath().c_str(), strlen(panelConfig.getModelPath().c_str()));
                //char path[128] = { PATH };
                const char* f_path = tex->texture->image->initFrom->ref;
                memcpy_s(path + strlen(path), 128 - strlen(path), f_path, strlen(f_path));
                char* image = (char*)imageLoadFromFile(path, &width, &height, &components);

                if (image) {
                    glCreateTextures(texture_type, 1, texture);
                    *texturesType = texture_type;
                    if (std::string::npos != std::string(path).find(".jpg", 0)) {
                        glTextureStorage2D(*texture, 1, GL_RGB8, width, height);
                        glTextureSubImage2D(*texture, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
                    }
                    if (std::string::npos != std::string(path).find(".jpeg", 0)) {
                        glTextureStorage2D(*texture, 1, GL_RGB8, width, height);
                        glTextureSubImage2D(*texture, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
                    }
                    if (std::string::npos != std::string(path).find(".png", 0)) {
                        glTextureStorage2D(*texture, 1, GL_RGBA8, width, height);
                        glTextureSubImage2D(*texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image);
                    }
                    stbi_image_free(image);
                }
            }
        }
        if (colordesc->color) {
            glm::vec4 rgba;
            rgba.r = colordesc->color->rgba.R;
            rgba.g = colordesc->color->rgba.G;
            rgba.b = colordesc->color->rgba.B;
            rgba.a = colordesc->color->rgba.A;
            *colors = rgba;
        }
    }
}



void proccessNode(AkNode* node, std::vector<Drawable>& primitives)
{
    if (node->geometry) {
        AkGeometry* geometry{ ak_instanceObjectGeom(node) };
        AkMesh* mesh{ (AkMesh*)ak_objGet(geometry->gdata) };
        if ((AkGeometryType)geometry->gdata->type) {
            if (mesh) {
                AkMeshPrimitive* ptr = mesh->primitive;
                while (ptr) {
                    Drawable primitive;
                    primitive.loadMatrix(node);
                    primitive.processMesh(ptr);
                    primitives.push_back(primitive);
                    ptr = ptr->next;
                }
            }
        }
    }
    if (node->chld) {
        proccessNode(node->chld, primitives);
    }
    if (node->next) {
        proccessNode(node->next, primitives);
    }
}

std::string printCoordSystem(AkCoordSys* coord) 
{
    if (coord) {
        AkAxis axis[] = {
            coord->axis.fwd,
            coord->axis.right,
            coord->axis.up,
            coord->cameraOrientation.fwd,
            coord->cameraOrientation.right,
            coord->cameraOrientation.up };
        std::string ax_name[] = { "axis FW:" ,"axis RH:" ,"axis UP:", "camera FW:", "camera RH:", "camera UP : " };

        AkAxisRotDirection axis_dir{ coord->rotDirection };
        std::string coordString;

        for (int i = 0; i < sizeof(axis) / sizeof(AkAxis); i++) {
            std::string st;
            switch (axis[i]) {
            case AK_AXIS_NEGATIVE_X: st = "NEGATIVE_X"; break;
            case AK_AXIS_NEGATIVE_Y: st = "NEGATIVE_Y"; break;
            case AK_AXIS_NEGATIVE_Z: st = "NEGATIVE_Z"; break;
            case AK_AXIS_POSITIVE_X: st = "POSITIVE_X"; break;
            case AK_AXIS_POSITIVE_Y: st = "POSITIVE_Y"; break;
            case AK_AXIS_POSITIVE_Z: st = "POSITIVE_Z"; break;
            }
            coordString += ax_name[i] + " " + st + "\n";
        }
        switch (axis_dir) {
        case AK_AXIS_ROT_DIR_LH: coordString += "rot dir: ROT LEFT\n";  break;
        case AK_AXIS_ROT_DIR_RH: coordString += "rot dir: ROT RIGHT\n";  break;
        }
        return coordString;
    }
    return "CoordSys is nullptr!\n";
}

std::string printDocInformation(AkDocInf* inf, AkUnit* unit) 
{
    std::string infString;
    if (inf && unit) {
        infString += "Units: " + std::string(unit->name) + " ";
        infString += unit->dist;
        infString += "\nPath: " + std::string(inf->name);
        infString += "\nFlip Image: ";
        infString += inf->flipImage ? "True" : "False";
        infString += "\n";
        if (AK_FILE_TYPE_GLTF == inf->ftype) {
            infString += "Type: GLTF\n";
        }
        else {
            infString += "Unknown type\n";
        }

        return infString;
    }
    return "AkDocInf or AkUnit is nullptr!\n";
}




void formatAttribute(GLint attr_location, AkAccessor* acc)
{
    int comp_size{ acc->componentSize };
    int type{ acc->componentType};
    GLint normalize{ acc->normalized ? GL_TRUE : GL_FALSE };
    //size_t offset = acc->byteOffset;
    //int comp_stride = acc->componentBytes;
    //size_t length = acc->byteLength;

    switch (comp_size) {
    case AK_COMPONENT_SIZE_SCALAR:                comp_size = 1; break;
    case AK_COMPONENT_SIZE_VEC2:                  comp_size = 2; break;
    case AK_COMPONENT_SIZE_VEC3:                  comp_size = 3; break;
    case AK_COMPONENT_SIZE_VEC4:                  comp_size = 4; break;
    case AK_COMPONENT_SIZE_MAT2:                  comp_size = 4; break;
    case AK_COMPONENT_SIZE_MAT3:                  comp_size = 9; break;
    case AK_COMPONENT_SIZE_MAT4:                  comp_size = 16; break;
    case AK_COMPONENT_SIZE_UNKNOWN:
    default:                                      comp_size = 1; break;
    }

    switch (type) {
    case AKT_FLOAT:						type = GL_FLOAT; break;
    case AKT_UINT:						type = GL_UNSIGNED_INT; break;
    case AKT_BYTE:						type = GL_BYTE; break;
    case AKT_UBYTE:						type = GL_UNSIGNED_BYTE; break;
    case AKT_SHORT:						type = GL_SHORT; break;
    case AKT_USHORT:					type = GL_UNSIGNED_SHORT; break;
    case AKT_UNKNOWN:
    case AKT_NONE:
    default:                            type = GL_INT; break;
    };

    glVertexAttribFormat(attr_location, comp_size, type, normalize, 0);
}

char* readFile(const char* file_name)
{
    FILE* fs;
    fopen_s(&fs, file_name, "rb");

    if (!fs) return nullptr;


    fseek(fs, 0, SEEK_END);
    int file_size{ ftell(fs) };
    rewind(fs);

    char* buffer{ (char*)calloc(file_size + 1, 1) };
    if (buffer) fread(buffer, 1, file_size, fs);
    fclose(fs);

    return buffer;
}

GLint checkPipelineStatus(GLuint vertex_shader, GLuint fragment_shader) 
{
    GLint v_comp_status, f_comp_status;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &v_comp_status);
    if (!v_comp_status) {
        char comp_info[1024] = {'\0'};
        //memset(comp_info, '\0', 1024);
        //glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, NULL);
        glGetShaderInfoLog(vertex_shader, 1024, NULL, comp_info);
        //std::cout << "Vertex Shader: ";
        //fwrite(comp_info, 1024, 1, stdout);
        logger.error(comp_info);
    }
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &f_comp_status);
    if (!f_comp_status) {
        char comp_info[1024] = { '\0' };
        //memset(comp_info, '\0', 1024);
        //glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, NULL);
        glGetShaderInfoLog(fragment_shader, 1024, NULL, comp_info);
        //std::cout << "Fragment Shader: ";
        //fwrite(comp_info, 1024, 1, stdout);
        logger.error(comp_info);
    }
    return (!v_comp_status || !f_comp_status) ? 0 : 1;
}



/* ============== Unused ================ */
void setPointer(GLuint program,
    GLint& mvpBindingLocation,
    GLint& vertexPosBindingLocation,
    GLint& vcol_location)
{
    mvpBindingLocation = glGetUniformLocation(program, "MVP");
    vertexPosBindingLocation = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

    glEnableVertexAttribArray(vertexPosBindingLocation);
    glEnableVertexAttribArray(vcol_location);

    glVertexAttribPointer(vertexPosBindingLocation, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 2));
}

void setPointer2(
    GLuint program,
    GLint& mvpBindingLocation,
    GLint& vertexPosBindingLocation,
    GLint& vcol_location)
{
    mvpBindingLocation = glGetUniformLocation(program, "MVP");
    vertexPosBindingLocation = glGetAttribLocation(program, "vPos");

    glEnableVertexAttribArray(vertexPosBindingLocation);
    glVertexAttribPointer(vertexPosBindingLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
}

void setPointer3(
    GLuint program,
    GLint& mvpBindingLocation,
    GLint& vertexPosBindingLocation,
    GLint& vcol_location)
{
    mvpBindingLocation = glGetUniformLocation(program, "MVP");
    vertexPosBindingLocation = glGetAttribLocation(program, "vPos");

    glVertexAttribFormat(vertexPosBindingLocation, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(vertexPosBindingLocation, 0);
    glEnableVertexAttribArray(vertexPosBindingLocation);
}
