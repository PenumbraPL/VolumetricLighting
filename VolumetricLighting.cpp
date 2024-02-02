// VolumetricLighting.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "VolumetricLighting.h"
#include "pch.h"
#include "Debug.h"
#include "Models.h";


ConfigContext panel_config{
    500.f, .001f, 50, 0, 0, 0, 0, 0, 50, 0, 0, false, false, { 0.4f, 0.7f, 0.0f, 0.5f }, { 0.4f, 0.7f, 0.0f, 0.5f },{ 0.4f, 0.7f, 0.0f, 0.5f }, { 0.0f, 0.0f, 0.0f }, 0.1, 0.5, 0.5
};
auto logger = spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt");


std::vector<PointLight> lights_list;


void init_lights(void) {
    lights_list.push_back({ glm::vec3(1.5, 1.5, 1.5), 0.1, 0.5, 0.5, glm::vec3(1., 1., 1.), glm::vec3(1., 1., 1.), glm::vec3(1., 1., 1.) });
    lights_list.push_back({ glm::vec3(-1.5, -1.5, 1.5), 0.1, 0.5, 0.5, glm::vec3(1., .9, .8), glm::vec3(.7, .5, .4), glm::vec3(.3, .2, .1) });
    lights_list.push_back({ glm::vec3(1.5, -1.5, 1.5), 0.1, 0.5, 0.5, glm::vec3(1., .9, .8), glm::vec3(.7, .5, .4), glm::vec3(.3, .2, .1) });
}

bool compare_lights(PointLight& old_light, PointLight& new_light) {
    return memcmp(&old_light, &new_light, sizeof(PointLight));
}

bool compare_lights(LightsList& old_light, LightsList& new_light) {
    if (old_light.size != new_light.size) return true;

    return memcmp(&old_light.list, &new_light.list, old_light.size * sizeof(PointLight));
}


void insert_tree(ConfigContext& context, std::vector<std::string>& tree) {
    context.directory = &tree;
}


GLint checkPipelineStatus(GLuint vertex_shader, GLuint fragment_shader) {
    GLint v_comp_status, f_comp_status;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &v_comp_status);
    if (!v_comp_status) {
        char comp_info[1024];
        memset(comp_info, '\0', 1024);
        //glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, NULL);
        glGetShaderInfoLog(vertex_shader, 1024, NULL, comp_info);
        std::cout << "Vertex Shader: ";
        fwrite(comp_info, 1024, 1, stdout);
    }
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &f_comp_status);
    if (!f_comp_status) {
        char comp_info[1024];
        memset(comp_info, '\0', 1024);
        //glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, NULL);
        glGetShaderInfoLog(fragment_shader, 1024, NULL, comp_info);
        std::cout << "Fragment Shader: ";
        fwrite(comp_info, 1024, 1, stdout);
    }
    return (!v_comp_status || !f_comp_status) ? 0 : 1;
}



WindowInfo windowConfig = {
    1900,
    1000,
    "GLTF Viewer",
    0, 0, 0
};

double xpos, ypos;
float mouse_speed = 2.f;
std::vector<glm::mat4x4*> mats;
glm::vec4 cam;
std::map <void*, unsigned int> bufferViews;
std::map <void*, unsigned int> textureViews;
std::map <void*, unsigned int> imageViews;
std::vector<Primitive> primitives;

std::vector<Light> lights;
std::vector<Camera> cameras;

namespace fs = std::filesystem;


/* ============================================================================= */


PointLight getLight(ConfigContext& panel_config) {
    glm::vec3 ambient = { panel_config.light_ambient[0], panel_config.light_ambient[1], panel_config.light_ambient[2] };
    glm::vec3 diffuse = { panel_config.light_diffuse[0], panel_config.light_diffuse[1], panel_config.light_diffuse[2] };
    glm::vec3 specular = { panel_config.light_specular[0], panel_config.light_specular[1], panel_config.light_specular[2] };
    glm::vec4 position = { panel_config.position[0], panel_config.position[1], panel_config.position[2], 1. };

    float l_position[16] = {};
    l_position[0] = 1.;
    l_position[5] = 1.;
    l_position[10] = 1.;
    l_position[15] = 1.;

    //glm::mat4 View = glm::rotate(
    //    glm::rotate(
    //        glm::translate(
    //            glm::make_mat4x4(l_position)
    //            , translate)
    //        , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
    //    rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
    //glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
    //glm::mat4 MVP = Projection * LookAt * View * Model;

    glm::vec4 new_position = position;

    return  { new_position, panel_config.c, panel_config.l, panel_config.q, ambient, diffuse, specular };
}


glm::vec3 getTranslate(ConfigContext& panel_config) {
    return  glm::vec3(panel_config.tr_x * 0.02, panel_config.tr_y * 0.02, panel_config.tr_z * 0.02);
}

glm::vec3 getRotate(ConfigContext& panel_config) {
    return glm::vec3(3.14 * panel_config.rot_x / 180, 3.14 * panel_config.rot_y / 180, 0.f);
}


glm::vec3 polar(ConfigContext& panel_config) {
    float r = 0.1 * panel_config.dist;
    float phi = panel_config.phi;
    float theta = panel_config.theta;
    glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));
    
    return glm::vec3(eye.z, eye.y, eye.x);
}


glm::mat4 preperLookAt(ConfigContext& panel_config){
    float theta = panel_config.theta;
    glm::vec3 eye = polar(panel_config); 

    glm::vec3 north = glm::vec3(0., 1., 0.);
    float corrected_theta = glm::fmod(glm::abs(theta), 6.28f);
    if (corrected_theta > 3.14 / 2. && corrected_theta < 3.14 * 3. / 2.) {
        north = glm::vec3(0., -1., 0.);
    }
    return  glm::lookAt(eye, glm::vec3(0.), north);
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

void set_up_color(AkColorDesc* colordesc, AkMeshPrimitive* prim, GLuint* sampler, GLuint* texture, GLuint* tex_type) {
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
                char path[128] = { PATH };
                const char* f_path = tex->texture->image->initFrom->ref;
                memcpy_s(path + strlen(path), 128 - strlen(path), f_path, strlen(f_path));
                char* image = (char*)imageLoadFromFile(path, &width, &height, &components);

                if (image) {
                    glCreateTextures(texture_type, 1, texture);
                    *tex_type = texture_type;
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




void proccess_node(AkNode* node) {
    int offset = 0;
    int comp_stride = 0;
    int normalize = 0;
    int type = 0;
    int comp_size = 0;

    Primitive pr;
    std::string geo_type;

    float* world_transform = pr.setWorldTransform();
    float* transform = pr.setTransform();
    pr.createSamplers();
    pr.createTextures();
    ak_transformCombineWorld(node, world_transform);
    ak_transformCombine(node, transform);
    std::regex light_regex("^[Ll]ight.*");

    if (node->geometry) {
        AkGeometry* geometry = ak_instanceObjectGeom(node); // if geometry
        AkMesh* mesh = (AkMesh*)ak_objGet(geometry->gdata);
        switch ((AkGeometryType)geometry->gdata->type) { //if gdata
        case AK_GEOMETRY_MESH:
            geo_type = "mesh";
            if (mesh) {
                GLuint prim_type;

                for (int i = 0; i < mesh->primitiveCount; i++) {/*prim = prim->next;*/ }
                AkMeshPrimitive* prim = mesh->primitive;
                switch (prim->type) {
                case AK_PRIMITIVE_LINES:              prim_type = GL_LINES; break;
                case AK_PRIMITIVE_POLYGONS:           prim_type = GL_POLYGON; break;
                case AK_PRIMITIVE_TRIANGLES:          prim_type = GL_TRIANGLES; break;
                case AK_PRIMITIVE_POINTS:
                default:                              prim_type = GL_POINTS; break;
                }
                if (prim->indices) {
                    pr.ind = (uint32_t*)prim->indices->items;
                    pr.ind_size = prim->indices->count;
                }
                std::cout << "Mesh name:" << mesh->name << std::endl;   // should i insert mesh->name ??
                std::cout << "Mesh center:" << mesh->center << std::endl; // same
                std::cout << "Primitive center: " << prim->center << std::endl;
                int set = prim->input->set;

                if (prim->material) {
                    AkMaterial* mat = prim->material;
                    AkEffect* ef = (AkEffect*)ak_instanceObject(&mat->effect->base);
                    AkTechniqueFxCommon* tch = ef->profile->technique->common;
                    if (tch) {
                        set_up_color(tch->ambient, prim, &pr.samplers[AMBIENT], &pr.textures[AMBIENT], &pr.tex_type[AMBIENT]);
                        set_up_color(tch->emission, prim, &pr.samplers[EMISIVE], &pr.textures[EMISIVE], &pr.tex_type[EMISIVE]);
                        set_up_color(tch->diffuse, prim, &pr.samplers[DIFFUSE], &pr.textures[DIFFUSE], &pr.tex_type[DIFFUSE]);
                        set_up_color(tch->specular, prim, &pr.samplers[SPECULAR], &pr.textures[SPECULAR], &pr.tex_type[SPECULAR]);

                        switch (tch->type) {
                        case AK_MATERIAL_METALLIC_ROUGHNESS: {
                            AkMetallicRoughness* mr = (AkMetallicRoughness*)tch;
                            AkColorDesc alb_cd;
                            AkColorDesc mr_cd;
                            alb_cd.color = &mr->albedo;
                            alb_cd.texture = mr->albedoTex;
                            mr_cd.color = &mr->albedo;//&mr->roughness;
                            mr_cd.texture = mr->metalRoughTex;
                            set_up_color(&alb_cd, prim, &pr.samplers[ALBEDO], &pr.textures[ALBEDO], &pr.tex_type[ALBEDO]);
                            set_up_color(&mr_cd, prim, &pr.samplers[MAT_ROUGH], &pr.textures[MAT_ROUGH], &pr.tex_type[MAT_ROUGH]);
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
                            set_up_color(&sg_cd, prim, &pr.samplers[SP_GLOSSINESS], &pr.textures[SP_GLOSSINESS], &pr.tex_type[SP_GLOSSINESS]);
                            set_up_color(&dif_cd, prim, &pr.samplers[SP_DIFFUSE], &pr.textures[SP_DIFFUSE], &pr.tex_type[SP_DIFFUSE]);
                            break;
                        }
                        };
                        std::cout << "Is double sized: " << (tch->doubleSided ? "True" : "False");
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

                pr.wgs = wgs ? wgs->accessor : nullptr;
                pr.jts = jts ? jts->accessor : nullptr;
                pr.pos = pos ? pos->accessor : nullptr;
                pr.tex = tex ? tex->accessor : nullptr;
                pr.nor = nor ? nor->accessor : nullptr;
                pr.col = col ? col->accessor : nullptr;
                pr.tan = tan ? tan->accessor : nullptr;

                pr.createPipeline();

                primitives.push_back(pr);
            };
            break;
        case AK_GEOMETRY_SPLINE: geo_type = "spline"; break;
        case  AK_GEOMETRY_BREP:  geo_type = "brep";   break;
        default:                 geo_type = "other";  break;
        };
    }
    else if (std::regex_match(node->name, light_regex)) {
        Light light;
        light.transform = glm::make_mat4x4(transform);
        light.w_transform = glm::make_mat4x4(world_transform);
        free(transform);
        free(world_transform);

        light.loadMesh();
        lights.push_back(light);
    }

    std::cout << "Node name: " << node->name << std::endl;
    std::cout << "Node type: " << geo_type << std::endl;

    if (node->next) {
        node = node->next;
        proccess_node(node);
    }
    if (node->chld) {
        node = node->chld;
        proccess_node(node);
    }
}




int main(void)
{
    logger->info("========== Initialization started ============================================\n");
    if (!glfwInit()) {
        logger->error("========== [GLFW]: Initialization failed =====================================\n");
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);


    GLFWwindow* window = glfwCreateWindow(windowConfig.width, windowConfig.height, windowConfig.title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        logger->warn("========== [GLFW]: Terminated ================================================\n");
        logger->error("========== [GLFW]: Window initialization failed ==============================\n");
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    
    glfwSetKeyCallback(window, key_callback);
    glfwSetErrorCallback(glew_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetWindowFocusCallback(window, focus_callback);

    initializeGLEW();
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    ImGui::StyleColorsDark();
    const char* glsl_version = "#version 450";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        logger->info("========== [GLFW]: Debug context initialize successful =======================\n");
        std::vector<DEBUGPROC> callbacks;
        gl_fill_callback_list(callbacks);
        gl_debug_init(callbacks);
    }  else {
        logger->warn("========== [GLFW]: Debug context initialize unsuccessful =====================\n");
    }
    

    GLuint vao, vertex_buffer;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);


    /* ================================================ */

    //ak_imageInitLoader(imageLoadFromFile, imageLoadFromMemory, imageFlipVerticallyOnLoad);



    AkDoc* doc;
    AkVisualScene* scene;
    AkCamera* camera = nullptr;
    AkInstanceGeometry* geometry;
    AkNode *root, *node_ptr;

    std::string scene_path = PATH;
    scene_path += FILE_NAME;
    if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
        logger->error("Document couldn't be loaded\n");
        exit(EXIT_FAILURE);
    }else {
        logger->info(printCoordSys(doc->coordSys));
        logger->info(printInf(doc->inf, doc->unit));
        logger->info("==============================================================================\n");
    }

    float* camera_mat = (float*)calloc(16, sizeof(float));
    float* camera_proj = (float*)calloc(16, sizeof(float));
    glm::mat4 Projection;
    glm::mat4 Camera;

    if (doc->scene.visualScene) {
        scene = (AkVisualScene*) ak_instanceObject(doc->scene.visualScene);
        logger->info("=============== Visual Scene loaded ====");
        if (scene->name) {
            logger->info("Scene name: ");
            logger->info(scene->name);
            logger->info("============\n");
        }

        ak_firstCamera(doc, &camera, camera_mat, camera_proj);
        if (camera) {
            Projection = glm::make_mat4x4(camera_proj);
            Camera = glm::make_mat4x4(camera_mat);
            for (int i = 0; i < 16; i++) {
                std::cout << camera_mat[i] << ", ";
                if (i % 4 == 3) std::cout << std::endl;
            }
        } else if (scene->cameras) {
            if (scene->cameras->first) {
                camera = (AkCamera*)ak_instanceObject(scene->cameras->first->instance);
            }
        }
        if (camera) std::cout << "Camera name: " << camera->name << std::endl;


        AkNode* node = ak_instanceObjectNode(scene->node);
        proccess_node(node); // pointer to pointer?
    }

    if (camera_mat) free(camera_mat);
    if (camera_proj) free(camera_proj);

    // choose shaders from compiled set
    //primitives[0].program = &program;


    int j;

    // What with and libimages ??
    j = 0;
    FListItem* i = doc->lib.images;
    if (i) {
        do {
            AkImage* img = (AkImage*)i->data;
            imageViews.insert({ {img, 0} });
            i = i->next;
        } while (i);
        for (auto& u : imageViews) {
            u.second = j++;
        }
    }


    j = 0;
    FListItem* t = doc->lib.textures;
    if (t) {
        do {
            AkTexture* tex = (AkTexture*)t->data;
            textureViews.insert({ {tex, 0} });
            t = t->next;
        } while (t);
        for (auto& u : textureViews) {
            u.second = j++;
        }
    }


    j = 0;
    FListItem* b = (FListItem*)doc->lib.buffers;
    if (b) {
        do {
            AkBuffer* buf = (AkBuffer*)b->data;
            bufferViews.insert({ {buf, 0} });
            b = b->next;
        } while (b);
        for (auto& u : bufferViews) {
            u.second = j++;
        }
    }


    /* ======================================================== */

   
    GLuint* buffers = (GLuint*) calloc(bufferViews.size(), sizeof(GLuint));
    glCreateBuffers(bufferViews.size(), buffers);
    for (auto &buffer : bufferViews) {
        unsigned int i = bufferViews[buffer.first];
        glNamedBufferData(buffers[i], ((AkBuffer*) buffer.first)->length, ((AkBuffer*) buffer.first)->data, GL_STATIC_DRAW);
    }
    
    for (auto& p : primitives) p.getLocation();

    for (auto& l : lights)    l.createPipeline();


    Environment env;
    env.loadMesh();
    env.createPipeline();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    Cloud cld;
    cld.loadMesh();
    cld.createPipeline(width, height);


    //glDepthRange(panel_config.near_plane, panel_config.far_plane);
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   
    GLuint lights_buffer;
    glGenBuffers(1, &lights_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lights_buffer);

    init_lights();
    int light_size = sizeof(PointLight) * lights_list.size();
    unsigned int l_size = lights_list.size();

    glNamedBufferData(lights_buffer, sizeof(LightsList) + light_size, NULL, GL_DYNAMIC_DRAW);
    glNamedBufferSubData(lights_buffer, offsetof(LightsList, size), sizeof(unsigned int), &l_size);
    glNamedBufferSubData(lights_buffer, offsetof(LightsList, list), light_size, lights_list.data());


    logger->info("===================== Main loop ==============================================\n");
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glEnable(GL_DEPTH_TEST); 
        //glEnable(GL_BLEND);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0., 1., 1., 1.0);


        glm::vec3 eye = polar(panel_config);
        glm::mat4 LookAt = preperLookAt(panel_config);
        if (!camera) Projection = glm::perspectiveFov((float)3.14 * panel_config.fov / 180, (float)width, (float)height, panel_config.near_plane, panel_config.far_plane);
        

        env.draw(width, height, Projection, camera);

        glm::vec3 translate = getTranslate(panel_config);
        glm::vec3 rotate = getRotate(panel_config); 
        PointLight new_light = getLight(panel_config);
        if (compare_lights(lights_list.data()[0], new_light)) {
            lights_list.data()[0] = new_light;
            LightsList* ptr = (LightsList*)glMapNamedBuffer(lights_buffer, GL_WRITE_ONLY);
            memcpy_s((void*)&ptr->list[0], sizeof(PointLight), &new_light, sizeof(PointLight));
            glUnmapNamedBuffer(lights_buffer);
        }


        for (auto& p : primitives) {
            p.draw(lights_buffer, bufferViews, buffers,
                eye, LookAt, Projection, translate, rotate);
        }
        cld.draw(width, height, Projection, camera, panel_config.g, lights_buffer);
        for (auto& l : lights)     l.drawLight(width, height, Projection, camera);



        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawLeftPanel(io, panel_config);
        drawRightPanel(io, panel_config);
        
        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glBindProgramPipeline(0);

    env.deletePipeline();
    cld.deletePipeline();

    for (auto& p : primitives) p.deleteTransforms();
    for (auto& p : primitives) p.deleteTexturesAndSamplers();
    for (auto& p : primitives) p.deletePipeline();
    for (auto& l : lights)     l.deletePipeline();


    glDeleteBuffers(bufferViews.size(), buffers); free(buffers);
    glDeleteVertexArrays(1, &vao);


    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    logger->info("========== [GLFW]: Terminated ================================================\n");
    logger->info("===================== Exit succeeded =========================================\n");
    return 0;
}
