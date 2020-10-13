// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <float.h>
#define EPSILON 0.0001
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <glm/ext.hpp>
//#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <algorithm>

#include "gmap.hpp"

glm::mat4 model_transfo(1.0f);

void rotate_x(float x = 0.01){
    vec3 EulerAngles(x, 0, 0);
    glm::mat4 m = glm::toMat4(quat(EulerAngles));
    model_transfo = m * model_transfo;
}

void rotate_y(float y = 0.01){
    vec3 EulerAngles(0, y, 0);
    glm::mat4 m = glm::toMat4(quat(EulerAngles));
    model_transfo = m * model_transfo;
}

/*******************************************************************************/

u_int pick_vertex(const GMap::idlist_t& sommets, const GMap::id_t source, const GMap3D& gmap){
    for(u_int i = 0 ; i < sommets.size() ; i++){ // Pour chaque 0-element:
        GMap::idlist_t orbit_elem_0 = gmap.orbit({1, 2}, sommets[i]); // Reconstituer son orbite directe (brins du vertex)
        if(std::count(orbit_elem_0.begin(), orbit_elem_0.end(), source) > 0){ // Si le brin de la face fait partie du 0-element
            return i; // On a trouve un vertex de la face
        }
    }
    return source;
}

void quad_to_triangles(std::vector<u_int>& quad){
    u_int d = quad.back();
    quad.pop_back();
    quad.push_back(quad[0]);
    quad.push_back(quad[2]);
    quad.push_back(d);
}

void transfert_temp_to_def(std::vector< std::vector<u_int> >& temp, std::vector<short unsigned int>& def){
    for(u_int i = 0 ; i < temp.size() ; i++){
        for(u_int k = 0 ; k < temp[i].size() ; k++){
            def.push_back(temp[i][k]);
        }
    }
}

void compute_normals(const std::vector<glm::vec3>& idx_vertices, const std::vector<unsigned short int>& ids, std::vector<glm::vec3>& normales, std::vector<float>& weights){
    normales.clear();
    for(u_int i = 0 ; i < ids.size() ; i += 3){
        glm::vec3 AB = idx_vertices[ids[i+1]] - idx_vertices[ids[i+0]];
        glm::vec3 AC = idx_vertices[ids[i+2]] - idx_vertices[ids[i+0]];
        glm::vec3 N = glm::cross(AB, AC);

        weights.push_back(glm::length(N));
        normales.push_back(glm::normalize(N));
    }
}

glm::vec3 sum(const std::vector<glm::vec3>& v){
    glm::vec3 accumulator(0.0, 0.0, 0.0);
    for(std::vector<glm::vec3>::const_iterator it = v.begin() ; it != v.end() ; it++){
        accumulator += (*it);
    }
    return accumulator;
}

void compute_vertices_normals(const std::vector<glm::vec3>& faces_normals,
                              const std::vector<short unsigned int>& indices,
                              const std::vector<glm::vec3>& indexed_vertices,
                              std::vector<glm::vec3>& indexed_normals,
                              const std::vector<float>& weights){

    std::vector< std::vector<glm::vec3> > accumulators(indexed_vertices.size(), std::vector<glm::vec3>());
    for(u_int i = 0 ; i < indices.size() ; i += 3){
        accumulators[indices[i+0]].push_back(faces_normals[i/3] * weights[i/3]);
        accumulators[indices[i+1]].push_back(faces_normals[i/3] * weights[i/3]);
        accumulators[indices[i+2]].push_back(faces_normals[i/3] * weights[i/3]);
    }

    indexed_normals.clear();

    for(u_int i = 0 ; i < accumulators.size() ; i++){
        indexed_normals.push_back(glm::normalize(sum(accumulators[i])));
    }

}

void swap(u_int& a, u_int& b){
    u_int c = a;
    a = b;
    b = c;
}

int display(const GMap3D& gmap)
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "TP3 - GMap", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "vertex_shader.glsl", "fragment_shader.glsl" );

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");


    std::vector<unsigned short> indices;
    std::vector<glm::vec3>      indexed_vertices;
    std::vector<glm::vec3>      indexed_normals;
    std::vector<unsigned int>   property;
    std::vector<float>          weights;
    std::vector<vec3>           faces_normals;


    GMap::idlist_t sommets = gmap.elements(0); // Liste des sommets
	GMap::idlist_t faces = gmap.elements(2);   // Liste des faces

    // Contiendra les faces indexées avant qu'elles ne soient découpées en triangles (si besoin)
    std::vector< std::vector<u_int> > temp_faces_indices(faces.size(), std::vector<u_int>());


    // # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
    // #           Recalage des valeurs entre -1.0 et 1.0                                    #
    // # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

    glm::vec3 p_temp = gmap.get_position(sommets[0]);
    float min = p_temp.x;
    float max = p_temp.x;

    // Recherche du min et du max
    for(u_int i = 0 ; i < sommets.size() ; i++){
        property.push_back(1); // ???
        glm::vec3 pos = gmap.get_position(sommets[i]);

        for(u_int k = 0 ; k < 3 ; k++){
            if(pos[k] < min){min = pos[k];}
            if(pos[k] > max){max = pos[k];}
        }

        indexed_vertices.push_back(pos); // On profite de la boucle pour récupérer les coordonnées de vertices
    }
    float val_norm = (glm::abs(min) > max) ? (glm::abs(min)) : (max); // Prend la plus grande valeur entre max et abs(min)

    for(u_int i = 0 ; i < indexed_vertices.size() ; i++){
        indexed_vertices[i] /= val_norm+5.0;
        std::cout << "Vertex[" << i << "] : " << indexed_vertices[i].x << " " << indexed_vertices[i].y << " " << indexed_vertices[i].z << std::endl;
    }
    std::cout << "* * * * * * * * * * *" << std::endl;

    // # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
    // #           Reconstitution des indices de faces :                                     #
    // # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #


    u_int j = 0; // Index du tableau de faces temporaire
    for(GMap::id_t face : faces){
        GMap::id_t i = (face % 2 > 0) ? (face) : (gmap.alpha({0}, face)); // S'assurer qu'on par d'un brin impair pour être en sens CCW
        GMap::id_t stop = i;  // Borne qui marque une boucle de face complète

        do{
            u_int id_sommet = pick_vertex(sommets, i, gmap); // Dans la boucle, permet de connaitre les 4 indices de vertices de la face
            temp_faces_indices[j].push_back(id_sommet); // Récup de l'index du vertex pour ce brin
            i = gmap.alpha({0, 1}, i); // Toujours dans le même sens
        }while(i != stop);

        j++;
    }


    // # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
    // #           Traitement des faces reconstituées :                                      #
    // # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #


    for(u_int i = 0 ; i < temp_faces_indices.size() ; i++){
        if(temp_faces_indices[i].size() > 3){
            quad_to_triangles(temp_faces_indices[i]); // Transformation des quads en triangles si pas déjà le cas
        }
    }

    transfert_temp_to_def(temp_faces_indices, indices); // Ecrit le tableau des indices de faces concaténés
    compute_normals(indexed_vertices, indices, faces_normals, weights); // Calcul des normales des FACES. Poids associé = aire de la face


    for(u_int i = 0 ; i < faces_normals.size() ; i++){ // Pour éviter les "-0" qui peuvent provoquer des bugs de normales
        if(glm::abs(faces_normals[i].x) < EPSILON){faces_normals[i].x = 0;}
        if(glm::abs(faces_normals[i].y) < EPSILON){faces_normals[i].y = 0;}
        if(glm::abs(faces_normals[i].z) < EPSILON){faces_normals[i].z = 0;}
        std::cout << "Face normal [" << i << "] : " << faces_normals[i].x << "  " << faces_normals[i].y << "  " << faces_normals[i].z << std::endl;
    }
    std::cout << "* * * * * * * * * * *" << std::endl;

    // Calcul des normales de vertices pondérées
    compute_vertices_normals(faces_normals, indices, indexed_vertices, indexed_normals, weights);

    for(u_int i = 0 ; i < indexed_normals.size() ; i++){
        std::cout << "Vertex normal [" << i << "] : " <<  indexed_normals[i].x << "  " << indexed_normals[i].y << "  " << indexed_normals[i].z << std::endl;
    }

    std::cout << "* * * * * * * * * * *" << std::endl;



    glm::vec3 bb_min( FLT_MAX, FLT_MAX, FLT_MAX );
    glm::vec3 bb_max( FLT_MIN, FLT_MIN, FLT_MIN );

    //Calcul de la boite englobante du modèle
    for( unsigned int i = 0 ; i < indexed_vertices.size() ; i++ ){
        bb_min = glm::min(bb_min, indexed_vertices[i]);
        bb_max = glm::max(bb_max, indexed_vertices[i]);
    }

    glm::vec3 size = bb_max - bb_min;
    glm::vec3 center = glm::vec3(bb_min.x + size.x/2, bb_min.y + size.y/2 , bb_min.z + size.z/2 );
    float model_scale = 2.0/std::max( std::max(size.x, size.y), size.z );


    //****************************************************************/

    // Load it into a VBO

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

    GLuint propertybuffer;
    glGenBuffers(1, &propertybuffer);
    glBindBuffer(GL_ARRAY_BUFFER, propertybuffer);
    glBufferData(GL_ARRAY_BUFFER, property.size() * sizeof(float), &property[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    do{
        if(glfwGetKey(window, GLFW_KEY_UP ) == GLFW_PRESS){
            rotate_x(0.1);
        }
        if(glfwGetKey(window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
            rotate_y(0.1);
        }
        if(glfwGetKey(window, GLFW_KEY_DOWN ) == GLFW_PRESS){
            rotate_x(-0.1);
        }
        if(glfwGetKey(window, GLFW_KEY_LEFT ) == GLFW_PRESS){
            rotate_y(-0.1);
        }
        // Measure speed
        double currentTime = glfwGetTime();
        nbFrames++;
        if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
            // printf and reset
            printf("%f ms/frame\n", 1000.0/double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);

        // Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
        // Camera matrix
        glm::mat4 ViewMatrix       = glm::lookAt(
                    glm::vec3(0,0,3), // Camera is at (4,3,3), in World Space
                    glm::vec3(0,0,0), // and looks at the origin
                    glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                    );
        // Model matrix : an identity matrix (model will be at the origin)
        glm::mat4 ModelMatrix      = glm::mat4(1.0f);///*glm::scale(glm::mat4(1.0f), glm::vec3(model_scale))**/glm::translate(glm::mat4(1.0f), glm::vec3(-center.x, -center.y, -center.z));
        ModelMatrix = model_transfo * ModelMatrix;
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

        glm::vec3 lightPos = glm::vec3(3, 3, 5);
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                    0,                  // attribute
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void*)0            // array buffer offset
                    );


        // 3rd attribute buffer : normals
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(
                    1,                                // attribute
                    3,                                // size
                    GL_FLOAT,                         // type
                    GL_FALSE,                         // normalized?
                    0,                                // stride
                    (void*)0                          // array buffer offset
                    );

        // 4th attribute buffer : property
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, propertybuffer);
        glVertexAttribPointer(
                    2,                                // attribute
                    1,                                // size
                    GL_FLOAT,                         // type
                    GL_FALSE,                         // normalized?
                    0,                                // stride
                    (void*)0                          // array buffer offset
                    );

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

        // Draw the triangles !
        glDrawElements(
                    GL_TRIANGLES,      // mode
                    indices.size(),    // count
                    GL_UNSIGNED_SHORT,   // type
                    (void*)0           // element array buffer offset
                    );

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteBuffers(1, &propertybuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
