/*----- shader reader -------
 * creates null terminated string from file ---*/
static char* readShaderSource(const char* shaderFile)
{   FILE* fp = fopen(shaderFile, "rb");
    char* buf;
    long size;

    if(fp==NULL) return NULL;
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    buf = (char*) malloc((size+1) * sizeof(char)); 
    fread(buf, 1, size, fp);
    buf[size] = '\0'; /* terminate the string with NULL */
    fclose(fp);
    return buf;
}

/*------ Standard GLSL initialization -----------
*** Usage example:
    initShader(&program1, "vmesh.glsl", "fPassThrough.glsl") ---

    ** Create a program object denoted by global var "program1" that uses 
                vertex shader file   "vmesh.glsl" and 
                fragment shader file "fPassThrough.glsl"
---------------------------------------*/
static void initShader(GLuint *programPtr,
                       const GLchar* vShaderFile, const GLchar* fShaderFile)
{   GLint status; 
    GLchar *vSource, *fSource;
    GLuint vShader, fShader; // for vertex and fragment shader handles
    GLuint program;          // for program handle
    GLchar *ebuffer; /* buffer for error messages */
    GLsizei elength; /* length of error message */

    /* read shader files */
    vSource = readShaderSource(vShaderFile);
    if(vSource==NULL)
    {
      printf("Failed to read vertex shader %s\n", vShaderFile);
        exit(EXIT_FAILURE);
    }
    else printf("Successfully read vertex shader %s\n", vShaderFile);

    fSource = readShaderSource(fShaderFile);
    if(fSource==NULL)
    {
      printf("Failed to read fragment shader %s\n", fShaderFile);
       exit(EXIT_FAILURE);
    }
    else printf("Successfully read fragment shader %s\n", fShaderFile);

    /* create program and shader objects */
    vShader = glCreateShader(GL_VERTEX_SHADER);
    fShader = glCreateShader(GL_FRAGMENT_SHADER);
    program = glCreateProgram();

    /* attach shaders to the program object */
    glAttachShader(program, vShader); 
    glAttachShader(program, fShader);

    /* read shaders */
    glShaderSource(vShader, 1, (const GLchar**) &vSource, NULL);
    glShaderSource(fShader, 1, (const GLchar**) &fSource, NULL);

    /* compile vertex shader */
    glCompileShader(vShader);

    /* error check */
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &status);
    if(status==GL_FALSE)
    {
      printf("Failed to compile vertex shader %s\n", vShaderFile);
       glGetShaderiv(vShader, GL_INFO_LOG_LENGTH, &elength);
       ebuffer = (GLchar *) malloc(elength*sizeof(char));
       glGetShaderInfoLog(vShader, elength, NULL, ebuffer);
       printf("%s\n", ebuffer); free(ebuffer);
      exit(EXIT_FAILURE);
    }
    else printf("Successfully compiled vertex shader %s\n", vShaderFile);

    /* compile fragment shader */
    glCompileShader(fShader);

    /* error check */
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &status);
    if(status==GL_FALSE)
    {
      printf("Failed to compile fragment shader %s\n", fShaderFile);
       glGetShaderiv(fShader, GL_INFO_LOG_LENGTH, &elength);
       ebuffer = (GLchar *) malloc(elength*sizeof(char));
       glGetShaderInfoLog(fShader, elength, NULL, ebuffer);
       printf("%s\n", ebuffer); free(ebuffer);
       exit(EXIT_FAILURE);
    }
    else printf("Successfully compiled fragment shader %s\n", fShaderFile);

    /* link and error check */
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(status==GL_FALSE)
    {
      printf("Failed to link program object\n");
       glGetProgramiv(program, GL_INFO_LOG_LENGTH, &elength);
       ebuffer = (GLchar *) malloc(elength*sizeof(char));
       glGetProgramInfoLog(program, elength, NULL, ebuffer);
       printf("%s\n", ebuffer); free(ebuffer);
       exit(EXIT_FAILURE);
    }
    else printf("Successfully linked program object\n\n");

    /*--- Return the created program handle to the 
          "output" function parameter "*programPtr" ---*/
    *programPtr = program; 
}
