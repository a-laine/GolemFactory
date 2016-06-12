/*
  ==================================
   Création d'une fenetre
   et affichage d'un triangle
  ==================================
*/

#include <iostream>
#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"





GLFWwindow* window;

void initialiseGLFW()
{
	// Initialise GLFW
	if(!glfwInit())
		exit(-1);

	glfwWindowHint(GLFW_SAMPLES, 4); // antialiasing 4x 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Nous voulons OpenGL 3.3 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Nous ne voulons pas de support de l'ancien OpenGL
	
	// Ouvre une fenêtre et crée son contexte OpenGL
	window = glfwCreateWindow(1024, 768, "Tuto OpenGL", NULL, NULL);
	if(!window)
	{
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);
}

void initialiseGLEW()
{
	// Initialise GLEW
	glewExperimental=true; // Nécessaire pour le profil core
	if(glewInit() != GLEW_OK)
		exit(-1);
}




int main(void)
{
	initialiseGLFW();
	initialiseGLEW();


	// S'assure que l'on puisse capturer la touche échap utilisée plus bas
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// ?
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Couleur background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);



	// ======================
	//  Création du modele
	// ======================
	// Un triangle
	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f,  1.0f, 0.0f,
	};
	// Ceci identifiera notre tampon de sommets
	GLuint vertexbuffer;
	// Génère un tampon et place l'identifiant dans 'vertexbuffer'
	glGenBuffers(1, &vertexbuffer);
	// Les commandes suivantes vont parler de notre tampon 'vertexbuffer'
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Fournit les sommets à OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// Crée et compile notre programme GLSL à partir des shaders
	GLuint programID = LoadShaders("SimpleTransform.vertexshader", "SimpleColor.fragmentshader");




	// =============================
	//  Matrices de transformations
	// =============================
	// Obtient un identifiant pour notre variable uniforme "MVP". 
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Matrice de projection : Champ de vision de 45° , ration 4:3, distance d'affichage : 0.1 unités <-> 100 unités 
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Matrice de la caméra
	glm::mat4 View       = glm::lookAt(
		glm::vec3(4, 3, 3), // La caméra est à (4,3,3), dans l'espace monde
		glm::vec3(0, 0, 0), // et regarde l'origine
		glm::vec3(0, 1, 0)  // La tête est vers le haut (utilisez 0,-1,0 pour regarder à l'envers) 
		);
	// Matrice de modèle : une matrice d'identité (le modèle sera à l'origine) 
	glm::mat4 Model      = glm::mat4(1.0f);  // Changez pour chaque modèle ! 
	// Notre matrice ModelViewProjection : la multiplication des trois  matrices 
	glm::mat4 MVP        = Projection * View * Model; // Souvenez-vous, la multiplication de matrice fonctionne dans l'autre sens





	// ===============
	//  Main loop
	// ===============

	// Loop until the user closes the window
	while(!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
	{
		// Render here :
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Utilise notre shader
			glUseProgram(programID);

			// Envoie notre transformation au shader actuel dans la variable uniforme "MVP" 
			// Pour chaque modèle affiché, comme la MVP sera différente (au moins pour la partie M)
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

			// premier tampon d'attributs : les sommets
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glVertexAttribPointer(
				0,                  // attribut 0. Aucune raison particulière pour 0, mais cela doit correspondre au « layout » dans le shader 
				3,                  // taille
				GL_FLOAT,           // type 
				GL_FALSE,           // normalisé ? 
				0,                  // nombre d'octets séparant deux sommets dans le tampon
				(void*) 0            // décalage du tableau de tampon
				);

			// Dessine le triangle ! 
			glDrawArrays(GL_TRIANGLES, 0, 3); // Démarre à partir du sommet 0; 3 sommets au total -> 1 triangle 

			glDisableVertexAttribArray(0);
		}

		{
			// Swap front and back buffers
			glfwSwapBuffers(window);
			// Poll for and process events
			glfwPollEvents();
		}
	}



	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	glfwTerminate();
	return 0;
}

