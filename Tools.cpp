#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "pch.h"
#include "Models.h"
#include "GUI.h"

extern spdlog::logger logger;


void initialize_GLEW(void) 
{
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        logger.error("========== [GLEW]: Initialization failed =====================================\n");
        std::string text = "\tError:";
        text += (const char*) glewGetErrorString(err);
        logger.error(text);
    }
    std::string text = "========== [GLEW]: Using GLEW ";
    text += (const char*)glewGetString(GLEW_VERSION);
    text += " =========================================\n";
    logger.info(text);

    // glewIsSupported supported from version 1.3
    if (GLEW_VERSION_1_3) {
        std::string versionName = "GL_VERSION_4_5";
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

                text = "========== [GLEW]: For " + versionName + " extension " + ext + " isn't supported \n";
                logger.warn(text);
            }
        }
    }
    else {
        logger.warn("========== [GLEW]: OpenGL's extensions support haven't been verified! ============================\n");
    }
}



GLuint wrap_mode(AkWrapMode& wrap) {
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

void set_up_color(
    AkColorDesc* colordesc,
    AkMeshPrimitive* prim,
    GLuint* sampler,
    GLuint* texture,
    GLuint* texturesType,
    ConfigContext& panelConfig) 
{
    if (colordesc) {
        if (colordesc->texture) {
            AkTextureRef* tex = colordesc->texture;
            if (tex->texture) {
                AkSampler* samp = tex->texture->sampler;
                if (!samp) return;

                AkTypeId type = tex->texture->type;

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
                wrap_t = wrap_mode(samp->wrapT);
                wrap_s = wrap_mode(samp->wrapS);
                wrap_p = wrap_mode(samp->wrapP);

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
                int components = 0;
                int width = 0;
                int height = 0;
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
    }
}




void proccess_node(AkNode* node, std::vector<Primitive>& primitives)
{
    Primitive primitive;
    std::string geo_type;

    float* world_transform = primitive.setWorldTransform();
    float* localTransform = primitive.setTransform();
    primitive.createSamplers();
    primitive.createTextures();
    ak_transformCombineWorld(node, world_transform);
    ak_transformCombine(node, localTransform);
    std::regex light_regex("^[Ll]ight.*");

    if (node->geometry) {
        AkGeometry* geometry = ak_instanceObjectGeom(node); // if geometry
        AkMesh* mesh = (AkMesh*)ak_objGet(geometry->gdata);
        switch ((AkGeometryType)geometry->gdata->type) { //if gdata
        case AK_GEOMETRY_MESH:
            geo_type = "mesh";
            if (mesh) {
                GLuint prim_type;

//                for (int i = 0; i < mesh->primitiveCount; i++) {/*prim = prim->next;*/ }
                AkMeshPrimitive* prim = mesh->primitive;
                switch (prim->type) {
                case AK_PRIMITIVE_LINES:              prim_type = GL_LINES; break;
                case AK_PRIMITIVE_POLYGONS:           prim_type = GL_POLYGON; break;
                case AK_PRIMITIVE_TRIANGLES:          prim_type = GL_TRIANGLES; break;
                case AK_PRIMITIVE_POINTS:
                default:                              prim_type = GL_POINTS; break;
                }
                if (prim->indices) {
                    primitive.ind = (uint32_t*)prim->indices->items;
                    primitive.ind_size = (unsigned int) prim->indices->count;
                }
                //std::cout << "Mesh name:" << mesh->name << std::endl;   // should i insert mesh->name ??
                //std::cout << "Mesh center:" << mesh->center << std::endl; // same
                //std::cout << "Primitive center: " << prim->center << std::endl;
                int set = prim->input->set;

                if (prim->material) {
                    AkMaterial* mat = prim->material;
                    AkEffect* ef = (AkEffect*)ak_instanceObject(&mat->effect->base);
                    AkTechniqueFxCommon* tch = ef->profile->technique->common;
                    if (tch) {
                        set_up_color(tch->ambient, prim, &primitive.samplers[AMBIENT], &primitive.textures[AMBIENT], &primitive.texturesType[AMBIENT], panelConfig);
                        set_up_color(tch->emission, prim, &primitive.samplers[EMISIVE], &primitive.textures[EMISIVE], &primitive.texturesType[EMISIVE], panelConfig);
                        set_up_color(tch->diffuse, prim, &primitive.samplers[DIFFUSE], &primitive.textures[DIFFUSE], &primitive.texturesType[DIFFUSE], panelConfig);
                        set_up_color(tch->specular, prim, &primitive.samplers[SPECULAR], &primitive.textures[SPECULAR], &primitive.texturesType[SPECULAR], panelConfig);

                        switch (tch->type) {
                        case AK_MATERIAL_METALLIC_ROUGHNESS: {
                            AkMetallicRoughness* mr = (AkMetallicRoughness*)tch;
                            AkColorDesc alb_cd;
                            AkColorDesc mr_cd;
                            alb_cd.color = &mr->albedo;
                            alb_cd.texture = mr->albedoTex;
                            mr_cd.color = &mr->albedo;//&mr->roughness;
                            mr_cd.texture = mr->metalRoughTex;
                            set_up_color(&alb_cd, prim, &primitive.samplers[ALBEDO], &primitive.textures[ALBEDO], &primitive.texturesType[ALBEDO], panelConfig);
                            set_up_color(&mr_cd, prim, &primitive.samplers[MAT_ROUGH], &primitive.textures[MAT_ROUGH], &primitive.texturesType[MAT_ROUGH], panelConfig);
                            break;
                        }

                        case AK_MATERIAL_SPECULAR_GLOSSINES: {
                            AkSpecularGlossiness* sg = (AkSpecularGlossiness*)tch;
                            AkColorDesc sg_cd;
                            AkColorDesc dif_cd;
                            sg_cd.color = &sg->specular;
                            sg_cd.texture = sg->specGlossTex;
                            dif_cd.color = &sg->diffuse;
                            dif_cd.texture = sg->diffuseTex;
                            set_up_color(&sg_cd, prim, &primitive.samplers[SP_GLOSSINESS], &primitive.textures[SP_GLOSSINESS], &primitive.texturesType[SP_GLOSSINESS], panelConfig);
                            set_up_color(&dif_cd, prim, &primitive.samplers[SP_DIFFUSE], &primitive.textures[SP_DIFFUSE], &primitive.texturesType[SP_DIFFUSE], panelConfig);
                            break;
                        }
                        };
                        //std::cout << "Is double sized: " << (tch->doubleSided ? "True" : "False");
                    }
                }

                AkInput* wgs = ak_meshInputGet(prim, "WEIGHTS", set);
                AkInput* jts = ak_meshInputGet(prim, "JOINTS", set);
                AkInput* pos = ak_meshInputGet(prim, "POSITION", set);
                AkInput* tex = ak_meshInputGet(prim, "TEXCOORD", set); // if indexed then multiple parts to proccess
                AkInput* nor = ak_meshInputGet(prim, "NORMAL", set);

                AkInput* col = ak_meshInputGet(prim, "COLOR", set);
                AkInput* tan = ak_meshInputGet(prim, "TANGENT", set);

                //std::cout << ak_meshInputCount(mesh) << std::endl;

                primitive.wgs = wgs ? wgs->accessor : nullptr;
                primitive.jts = jts ? jts->accessor : nullptr;
                primitive.pos = pos ? pos->accessor : nullptr;
                primitive.tex = tex ? tex->accessor : nullptr;
                primitive.nor = nor ? nor->accessor : nullptr;
                primitive.col = col ? col->accessor : nullptr;
                primitive.tan = tan ? tan->accessor : nullptr;

                primitive.createPipeline();

                primitives.push_back(primitive);
            };
            break;
        case AK_GEOMETRY_SPLINE: geo_type = "spline"; break;
        case  AK_GEOMETRY_BREP:  geo_type = "brep";   break;
        default:                 geo_type = "other";  break;
        };
    }
    /*else if (std::regex_match(node->name, light_regex)) {
        Light light;
        light.localTransform = glm::make_mat4x4(localTransform);
        light.worldTransform = glm::make_mat4x4(world_transform);
        free(localTransform);
        free(world_transform);

        light.loadMesh();
        lights.push_back(light);
        // light from gltf file
    }*/

    //std::cout << "Node name: " << node->name << std::endl;
    //std::cout << "Node type: " << geo_type << std::endl;

    if (node->next) {
        node = node->next;
        proccess_node(node, primitives);
    }
    if (node->chld) {
        node = node->chld;
        proccess_node(node, primitives);
    }
}




std::string print_coord_system(AkCoordSys* coord) 
{
    if (coord) {
        AkAxis axis[] = { coord->axis.fwd,
        coord->axis.right,
        coord->axis.up,
        coord->cameraOrientation.fwd,
        coord->cameraOrientation.right,
        coord->cameraOrientation.up };
        std::string ax_name[] = { "axis FW:" ,"axis RH:" ,"axis UP:", "camera FW:", "camera RH:", "camera UP : " };

        AkAxisRotDirection axis_dir = coord->rotDirection;
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

std::string print_doc_information(AkDocInf* inf, AkUnit* unit) 
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




void format_attribute(GLint attr_location, AkAccessor* acc) 
{
    int comp_size = acc->componentSize;;
    int type = acc->componentType;
    GLuint normalize = acc->normalized ? GL_TRUE : GL_FALSE;
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

char* read_file(const char* file_name) 
{
    FILE* fs;
    fopen_s(&fs, file_name, "rb");

    if (!fs) return nullptr;


    fseek(fs, 0, SEEK_END);
    int file_size = ftell(fs);
    rewind(fs);

    char* buffer = (char*)calloc(file_size + 1, 1);
    if (buffer) fread(buffer, 1, file_size, fs);
    fclose(fs);

    return buffer;
}

GLint check_pipeline_status(GLuint vertex_shader, GLuint fragment_shader) 
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
