enum eVertexArrayObject {
	VAOVerticesData,
	VAOCount
};
enum eBufferObject {
	VBOVerticesData,
	BOCount
};
enum eProgram {
	QuadScreenProgram,
	ProgramCount
};
enum eTexture {
	NoTexture,		
	TextureCount
};

#include <common.cpp>

GLint locationColor;
GLchar				windowTitle[] = "Drag-and-Drop";
GLfloat				aspectRatio;
GLint				dragged = -1;
static vector<vec2>	verticesData = {
	vec2(-0.5f, -0.5f),
	vec2(-0.5f,  0.5f),
	vec2(0.5f,  0.5f),
	vec2(0.5f, -0.5f)
};
void updateBuffer()
{
	glBindBuffer(GL_ARRAY_BUFFER, BO[VBOVerticesData]);
	glBufferData(GL_ARRAY_BUFFER,
		verticesData.size() * sizeof(vec2),
		verticesData.data(),
		GL_DYNAMIC_DRAW);
}
void initShaderProgram() {
	locationColor = glGetUniformLocation(program[QuadScreenProgram], "uColor");
	ShaderInfo shader_info[ProgramCount][3] = { {
		{ GL_VERTEX_SHADER,		"./vertexShader.glsl" },
		{ GL_FRAGMENT_SHADER,	"./fragmentShader.glsl" },
		{ GL_NONE, nullptr } } };

	for (int programItem = 0; programItem < ProgramCount; programItem++) {
		program[programItem] = LoadShaders(shader_info[programItem]);
		locationMatModel = glGetUniformLocation(program[programItem], "matModel");
		locationMatView = glGetUniformLocation(program[programItem], "matView");
		locationMatProjection = glGetUniformLocation(program[programItem], "matProjection");
	}
	locationColor = glGetUniformLocation(program[QuadScreenProgram], "uColor");
	glUseProgram(program[QuadScreenProgram]);
	glBindVertexArray(VAO[VAOVerticesData]);
	glBindBuffer(GL_ARRAY_BUFFER, BO[VBOVerticesData]);
	glBufferData(GL_ARRAY_BUFFER, verticesData.size() * sizeof(vec2), verticesData.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);
	glUseProgram(program[QuadScreenProgram]);
	matModel = mat4(1.0);
	matView = lookAt(
		vec3(0.0f, 0.0f, 9.0f),		
		vec3(0.0f, 0.0f, 0.0f),		
		vec3(0.0f, 1.0f, 0.0f));	
	glUniformMatrix4fv(locationMatModel, 1, GL_FALSE, value_ptr(matModel));
	glUniformMatrix4fv(locationMatView, 1, GL_FALSE, value_ptr(matView));
	glUniformMatrix4fv(locationMatProjection, 1, GL_FALSE, value_ptr(matProjection));
}

GLfloat distanceSquare(vec2 p1, vec2 p2) {
	vec2		delta = p1 - p2;
	return dot(delta, delta);		
}
GLint getActivePoint(vector<vec2> p, GLfloat sensitivity, vec2 mousePosition) {
	GLfloat		sensitivitySquare	= sensitivity * sensitivity;
	for (GLint i = 0; i < p.size(); i++)
		if (distanceSquare(p[i], mousePosition) < sensitivitySquare)
			return i;
	return -1;
}
vec2 Bezier(float t)
{
	int n = verticesData.size() - 1;
	vec2 p(0, 0);

	for (int i = 0; i <= n; i++)
	{
		float bin = 1;
		for (int j = 1; j <= i; j++)
			bin = bin * (n - (i - j)) / j;

		float B = bin * pow(1 - t, n - i) * pow(t, i);
		p += B * verticesData[i];
	}

	return p;
}

void DrawBezier()
{
	vector<vec2> curvePoints;

	for (float t = 0; t <= 1; t += 0.01f)
		curvePoints.push_back(Bezier(t));

	glBindBuffer(GL_ARRAY_BUFFER, BO[VBOVerticesData]);
	glBufferData(GL_ARRAY_BUFFER,
		curvePoints.size() * sizeof(vec2),
		curvePoints.data(),
		GL_DYNAMIC_DRAW);

	glUniform3f(locationColor, 0, 1, 0);
	glDrawArrays(GL_LINE_STRIP, 0, curvePoints.size());

	updateBuffer(); 
}
void display(GLFWwindow* window, double currentTime)
{
	glClear(GL_COLOR_BUFFER_BIT);
	updateBuffer();
	glUniform3f(locationColor, 0, 0, 1);
	glDrawArrays(GL_LINE_STRIP, 0, verticesData.size());
	glUniform3f(locationColor, 1, 0, 0);
	glDrawArrays(GL_POINTS, 0, verticesData.size());
	DrawBezier();
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	windowWidth = glm::max(width, 1);
	windowHeight = glm::max(height, 1);

	aspectRatio = (float)windowWidth / (float)windowHeight;
	glViewport(0, 0, windowWidth, windowHeight);
	if (projectionType == Orthographic)
		if (windowWidth < windowHeight)
			matProjection = ortho(-worldSize, worldSize, -worldSize / aspectRatio, worldSize / aspectRatio, -100.0, 100.0);
		else
			matProjection = ortho(-worldSize * aspectRatio, worldSize * aspectRatio, -worldSize, worldSize, -100.0, 100.0);
	else
		matProjection = perspective(
			radians(45.0f),	
			aspectRatio,	
			0.1f,			
			100.0f			
		);
	glUniformMatrix4fv(locationMatProjection, 1, GL_FALSE, value_ptr(matProjection));
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GLFW_TRUE);
	if (action == GLFW_PRESS)
		keyboard[key] = GL_TRUE;
	else if (action == GLFW_RELEASE)
		keyboard[key] = GL_FALSE;
	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		projectionType = Orthographic;
		framebufferSizeCallback(window, windowWidth, windowHeight);
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		projectionType = Perspective;
		framebufferSizeCallback(window, windowWidth, windowHeight);
	}
}
void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
	if (dragged >= 0) {
		dvec2	mousePosition;
		cout << "cursorPosCallback\t\t\t" << xPos << "\t" << yPos << endl;
		mousePosition.x = xPos * 2.0f / (GLdouble)windowWidth - 1.0f;
		mousePosition.y = ((GLdouble)windowHeight - yPos) * 2.0f / (GLdouble)windowHeight - 1.0f;
		if (windowWidth < windowHeight)
			mousePosition.y /= aspectRatio;
		else
			mousePosition.x *= aspectRatio;
		cout << "cursorPosCallback normalized coords\t" << mousePosition.x << "\t" << mousePosition.y << endl;
		verticesData.at(dragged) = mousePosition;
		verticesData[dragged] = mousePosition;
		glBindBuffer(GL_ARRAY_BUFFER, BO[VBOVerticesData]);
		glBufferData(GL_ARRAY_BUFFER, verticesData.size() * sizeof(vec2), verticesData.data(), GL_STATIC_DRAW);
	}
}
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		dvec2 mousePosition;
		glfwGetCursorPos(window, &mousePosition.x, &mousePosition.y);

		mousePosition.x = mousePosition.x * 2.0f / (GLdouble)windowWidth - 1.0f;
		mousePosition.y = ((GLdouble)windowHeight - mousePosition.y) * 2.0f / (GLdouble)windowHeight - 1.0f;

		if (windowWidth < windowHeight)
			mousePosition.y /= aspectRatio;
		else
			mousePosition.x *= aspectRatio;

		dragged = getActivePoint(verticesData, 0.1f, mousePosition);
		if (dragged == -1)
		{
			verticesData.push_back(mousePosition);
			updateBuffer();
		}

	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		dragged = -1; 
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		if (!verticesData.empty())
		{
			verticesData.pop_back();
			updateBuffer();
		}
	}
}

int main(void) {
	init(3, 3, GLFW_OPENGL_COMPAT_PROFILE);
	initShaderProgram();
	framebufferSizeCallback(window, windowWidth, windowHeight);
	while (!glfwWindowShouldClose(window)) {
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	cleanUpScene(EXIT_SUCCESS);
	return EXIT_SUCCESS;
}
