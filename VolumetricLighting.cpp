// VolumetricLighting.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "VolumetricLighting.h"
#define PATH "./res/Cube/"
#define FILE_NAME "cube.gltf"

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

/* ============================================================================= */

void print_map(const std::map<void*, unsigned int>& m){
      for (const auto& n : m)
          std::cout << n.first << " = " << n.second << "; ";
    std::cout << '\n';
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
                    if(std::string::npos != std::string(path).find(".jpeg", 0)){
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


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    panel_config.dist += yoffset * panel_config.dist / -6.;
}

static void cursor_position_callback(GLFWwindow* window, double new_xpos, double new_ypos)
{
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (state != GLFW_RELEASE)
    {
        double nx = (mouse_speed / windowConfig.width) * (new_xpos - xpos);
        double ny = (mouse_speed / windowConfig.height) * (new_ypos - ypos);
        panel_config.phi += nx;
        panel_config.theta += ny;
        
        xpos = new_xpos;
        ypos = new_ypos;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        glfwGetCursorPos(window, &xpos, &ypos);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.phi += 0.01;
    }
    if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.phi -= 0.01;
    }
    if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.theta += 0.01;
    }
    if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.theta -= 0.01;
    }
    if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.tr_z += 1;
    }
    if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.tr_z -= 1;
    }
    if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.tr_x -= 1;
    }
    if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.tr_x += 1;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        int mode[2] = { GLFW_CURSOR_DISABLED, GLFW_CURSOR_NORMAL };
        windowConfig.cursor_mode += 1;
        windowConfig.cursor_mode %= 2;
        glfwSetInputMode(window, GLFW_CURSOR, mode[windowConfig.cursor_mode % (sizeof(mode)/sizeof(int))]);
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
                            
                        case AK_MATERIAL_SPECULAR_GLOSSINES:{
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

std::string printCoordSys(AkCoordSys* coord) {
    if (coord) {
        AkAxis axis[] = { coord->axis.fwd,
        coord->axis.right,
        coord->axis.up,
        coord->cameraOrientation.fwd,
        coord->cameraOrientation.right,
        coord->cameraOrientation.up };
        std::string ax_name[] = { "axis FW:" ,"axis RH:" ,"axis UP:", "camera FW:", "camera RH:", "camera UP : "};

        AkAxisRotDirection axis_dir = coord->rotDirection;
        std::string coordString;

        for (int i = 0; i < sizeof(axis)/sizeof(AkAxis); i++) {
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

std::string printInf(AkDocInf* inf, AkUnit* unit) {
    std::string infString;
    if (inf && unit) {
        infString += "Units: " + std::string(unit->name) + " ";
        infString += unit->dist;
        infString += "\nPath: " + std::string(inf->name);
        infString += "\nFlip Image: ";
        infString += inf->flipImage ? "True" : "False";
        infString+= "\n";
        if (AK_FILE_TYPE_GLTF == inf->ftype) {
            infString += "Type: GLTF\n";
        }else {
            infString += "Unknown type\n";
        }
        
        return infString;
    }
    return "AkDocInf or AkUnit is nullptr!\n";
}




int main(void)
{
    std::cout << "========== Initialization started ============================================\n";
    if (!glfwInit()) {
        std::cout << "========== [GLFW]: Initialization failed =====================================\n";
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);


    GLFWwindow* window = glfwCreateWindow(windowConfig.width, windowConfig.height, windowConfig.title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        std::cout << "========== [GLFW]: Terminated ================================================\n";
        std::cout << "========== [GLFW]: Window initialization failed ==============================\n";
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    
    glfwSetKeyCallback(window, key_callback);
    glfwSetErrorCallback(error_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    initializeGLEW();
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    const char* glsl_version = "#version 450";
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        std::cout << "========== [GLFW]: Debug context initialize successful =======================\n";
        std::vector<DEBUGPROC> callbacks;
        callback_list(callbacks);
        debug_init(callbacks);
    }  else {
        std::cout << "========== [GLFW]: Debug context initialize unsuccessful =====================\n";
    }
    

    GLuint vao, vertex_buffer;
    GLint mvp_location, vpos_location, vcol_location, 
        norm_location, tex_location, is_tex_location, prj_location, camera_location;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);



    


    /* ================================================ */

    ak_imageInitLoader(imageLoadFromFile, imageLoadFromMemory, imageFlipVerticallyOnLoad);



    AkDoc* doc;
    AkVisualScene* scene;
    AkCamera* camera = nullptr;
    AkInstanceGeometry* geometry;
    AkNode *root, *node_ptr;

    std::string scene_path = PATH;
    scene_path += FILE_NAME;
    if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
        std::cout << "Document couldn't be loaded\n";
        return -1;
    }else {
        std::cout << printCoordSys(doc->coordSys);
        std::cout << printInf(doc->inf, doc->unit);
        std::cout << "==============================================================================\n";
    }

    float* camera_mat = (float*)calloc(16, sizeof(float));
    float* camera_proj = (float*)calloc(16, sizeof(float));
    glm::mat4 Projection;
    glm::mat4 Camera;

    if (doc->scene.visualScene) {
        scene = (AkVisualScene*) ak_instanceObject(doc->scene.visualScene);
        std::cout << "=============== Visual Scene loaded ====";
        if(scene->name) std::cout << "Scene name: " << scene->name << "============" << std::endl;
        

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
        print_map(bufferViews);
    }


    /* ======================================================== */

    //mvp_location = glGetUniformLocation(program, "MVP");
    //norm_location = glGetAttribLocation(program, "vNor");
    //vpos_location = glGetAttribLocation(program, "vPos");
    //tex_location = glGetAttribLocation(program, "vTex");
    //is_tex_location = glGetUniformLocation(program, "isTexture");


   
    GLuint* buffers = (GLuint*) calloc(bufferViews.size(), sizeof(GLuint));
    glCreateBuffers(bufferViews.size(), buffers);
    for (auto &buffer : bufferViews) {
        unsigned int i = bufferViews[buffer.first];
        glNamedBufferData(buffers[i], ((AkBuffer*) buffer.first)->length, ((AkBuffer*) buffer.first)->data, GL_STATIC_DRAW);
    }
    
    for (int i = 0; i < primitives.size(); i++) {
        GLuint& vertex_program = primitives[i].programs[VERTEX];
        GLuint& fragment_program = primitives[i].programs[FRAGMENT];
        // TO DO
        mvp_location = glGetUniformLocation(vertex_program, "MVP");
        norm_location = glGetAttribLocation(vertex_program, "vNor");
        vpos_location = glGetAttribLocation(vertex_program, "vPos");
        tex_location = glGetAttribLocation(vertex_program, "vTex");
        is_tex_location = glGetUniformLocation(fragment_program, "isTexture");
        prj_location = glGetUniformLocation(vertex_program, "PRJ");
        camera_location = glGetUniformLocation(fragment_program, "camera");


        if (mvp_location != -1) glEnableVertexAttribArray(mvp_location);
        if (vpos_location != -1) glEnableVertexAttribArray(vpos_location);
        if (norm_location != -1) glEnableVertexAttribArray(norm_location);
        if (tex_location != -1) glEnableVertexAttribArray(tex_location);
        if (is_tex_location != -1) glEnableVertexAttribArray(is_tex_location);
        if (prj_location != -1) glEnableVertexAttribArray(prj_location);
        if (camera_location != -1) glEnableVertexAttribArray(camera_location);


        if (vpos_location != -1) formatAttribute(vpos_location, primitives[i].pos);
        if (norm_location != -1) formatAttribute(norm_location, primitives[i].nor);
        if (tex_location  != -1) formatAttribute(tex_location, primitives[i].tex);
        //glObjectLabel(GL_BUFFER, buffers[binding_point], -1, "Vertex Buffer");
    }


    for (auto& l : lights) {
        l.createPipeline();
    }

    Environment env;
    env.loadMesh();
    env.createPipeline();

    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::cout << "===================== Main loop ==============================================\n";
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glEnable(GL_DEPTH_TEST); 
        glEnable(GL_BLEND);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0., 1., 1., 1.0);

        if (!camera) Projection = glm::perspectiveFov((float)3.14 * panel_config.fov / 180, (float)width, (float)height, panel_config.near_plane, panel_config.far_plane);

        env.draw(width, height, Projection, camera);




        for (int i = 0; i < primitives.size(); i++) {
            float r     = 0.1 * panel_config.dist;
            float phi   = panel_config.phi;
            float theta = panel_config.theta;

            glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));
            eye = glm::vec3(eye.z, eye.y, eye.x);
            
            glm::vec3 north = glm::vec3(0., 1., 0.);
            float corrected_theta = glm::fmod(glm::abs(theta), 6.28f);
            if (corrected_theta > 3.14/2. && corrected_theta < 3.14 * 3./2.) {
                north = glm::vec3(0., -1., 0.);
            }
            
            glm::vec3 translate = glm::vec3(panel_config.tr_x * 0.1, panel_config.tr_y * 0.1, panel_config.tr_z * 0.1);
            glm::vec3 rotate = glm::vec3(3.14 * panel_config.rot_x / 180, 3.14 * panel_config.rot_y / 180, 0.f);

            glBindProgramPipeline(primitives[i].pipeline);

            glm::mat4 LookAt = glm::lookAt(eye, glm::vec3(0.), north);
            if(!camera) Projection = glm::perspectiveFov((float) 3.14*panel_config.fov/180, (float) width, (float) height, panel_config.near_plane, panel_config.far_plane);

            glm::mat4 View =  glm::rotate(
                                glm::rotate(
                                    glm::translate(
                                        glm::make_mat4x4(primitives[i].transform)
                                        , translate)
                                    , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
                                    rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
            glm::mat4 MVP = LookAt * View * Model;
            glm::vec3 camera_view = glm::vec4(eye, 1.0);
            glProgramUniformMatrix4fv(primitives[i].programs[VERTEX], mvp_location, 1, GL_FALSE, glm::value_ptr(MVP));
            glProgramUniformMatrix4fv(primitives[i].programs[VERTEX], prj_location, 1, GL_FALSE, glm::value_ptr(Projection));
            glProgramUniform3fv(primitives[i].programs[FRAGMENT], camera_location, 1, glm::value_ptr(camera_view));


            if(!primitives[i].textures[ALBEDO])
                glProgramUniform1ui(primitives[i].programs[FRAGMENT], is_tex_location, GL_FALSE);

            int j = bufferViews[primitives[i].pos->buffer];
            int binding_point = 0;
            glVertexAttribBinding(vpos_location, binding_point);
            glBindVertexBuffer(binding_point, buffers[j], primitives[i].pos->byteOffset, primitives[i].pos->componentBytes);
            
            if (norm_location != -1) {
                j = bufferViews[primitives[i].nor->buffer];
                binding_point = 1;
                glVertexAttribBinding(norm_location, binding_point);
                glBindVertexBuffer(binding_point, buffers[j], primitives[i].nor->byteOffset, primitives[i].nor->componentBytes);
            }

            if (tex_location != -1) {
                j = bufferViews[primitives[i].tex->buffer];
                binding_point = 2;
                glVertexAttribBinding(tex_location, binding_point);
                glBindVertexBuffer(binding_point, buffers[j], primitives[i].tex->byteOffset, primitives[i].tex->componentBytes);
            }
            for (unsigned int tex_type = AMBIENT; tex_type < SIZE; tex_type++) {
                GLuint sampler = primitives[i].samplers[(TextureType) tex_type];
                GLuint texture = primitives[i].textures[(TextureType) tex_type];
                if (sampler && texture) {
                    //glProgramUniform1ui(primitives[i].programs[FRAGMENT], is_tex_location, GL_TRUE);
                    glBindSampler(tex_type, sampler);
                    glActiveTexture(GL_TEXTURE0 + tex_type);
                    glBindTexture(primitives[i].tex_type[tex_type], texture);
                }
                else {
                    glActiveTexture(GL_TEXTURE0 + tex_type);
                    glBindTexture(GL_TEXTURE_2D, 0); // needs change
                }
                
                /*
                else {
                    glProgramUniform1ui(primitives[i].programs[FRAGMENT], is_tex_location, GL_FALSE);
                }*/
            }

            glDrawElements(GL_TRIANGLES, primitives[i].ind_size, GL_UNSIGNED_INT, primitives[i].ind);
        }

        for (auto& l : lights)     l.drawLight(width, height, Projection, camera);

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawLeftPanel(io);
        drawRightPanel(io, panel_config);
        
        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glBindProgramPipeline(0);

   // env.deletePipeline();

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
    std::cout << "========== [GLFW]: Terminated ================================================\n";
    std::cout << "===================== Exit succeeded =========================================\n";
    return 0;
}