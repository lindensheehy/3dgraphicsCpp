#define SDL_MAIN_HANDLED

#include "src/include/Drawer.h"
#include "src/include/State.h"
#include "src/include/Camera.h"
#include "src/include/Mesh.h"
#include "src/include/Gui.h"
#include "src/include/UI.h"
#include "src/include/Log.h"

// Global declarations
Gui* gui;
State* state;
Drawer* drawer;
Display* display;
Camera* camera;

int selectedObjectId;
Object* selectedObject;

ObjectSet* objects;

bool drawNormals;
bool gravity;

UI* ui;

void handleInput(State* state, Camera* camera) {

    /*
        ---  Directional Movement  ---

        The camera movement depends on facing direction for x,z movement (using WASD), while the up and down movement do not change based on facing direction
        First a value 'dist' is found which determines how far the camera should move based on the dt (delta time)
        Then the up and down movement are simple because the camera y position is just added or subtracted by the dist amount
        The x,z movement is a bit more complicated because it uses a 2d vector to hold the actual movement where the x and y components hold the left-right and front-back
        then rotates it based on the camera yaw and adds that to position.
    */

    // Find distance to move based on the delta time of the frame
    double dist = camera->movementSpeed * (state->time->dt / 1000);

    if (state->keyIsDown(SDLK_LSHIFT))
        dist *= camera->sprintFactor;

    // Vertical Movement
    if (state->keyIsDown(SDLK_SPACE)) camera->pos->y += dist; // Up
    if (state->keyIsDown(SDLK_LCTRL)) camera->pos->y -= dist; // Down

    // Horizontal Movement
    Vec2* cameraMovVec = new Vec2();
    if (state->keyIsDown(SDLK_w)) cameraMovVec->y += dist; // Forward
    if (state->keyIsDown(SDLK_s)) cameraMovVec->y -= dist; // Backward
    if (state->keyIsDown(SDLK_a)) cameraMovVec->x -= dist; // Left
    if (state->keyIsDown(SDLK_d)) cameraMovVec->x += dist; // Right

    // Move camera based on its rotation
    cameraMovVec->rotate(-camera->yaw);
    camera->pos->x += cameraMovVec->x;
    camera->pos->z += cameraMovVec->y;
    delete cameraMovVec;

    /*  ---  Camera Rotation  ---  */
    if (state->mouse->leftButtonIsDown) {

        // 0.2 is just a random number I chose becuase it felt good in the app
        double mouseSensitivity = 0.2;
        double camDeltaYaw = (double) state->deltaMousePosX();
        double camDeltaPitch = (double) state->deltaMousePosY();
        camera->rotate( camDeltaYaw * mouseSensitivity, -camDeltaPitch * mouseSensitivity, 0 );

    }


    /*   Temporary stuff   */

    // Toggle normal vector drawing on key n
    if (state->keyJustDown(SDLK_n))
        drawNormals = !drawNormals;

    // Pick object based on id, this cycles through all the objects with ids 1-7
    bool changeObject = false;

    if (state->keyJustDown(SDLK_q)) {
        selectedObjectId--;
        if (selectedObjectId < 1) selectedObjectId += 8;
        changeObject = true;
    }

    if (state->keyJustDown(SDLK_e)) {
        selectedObjectId++;
        if (selectedObjectId > 8) selectedObjectId -= 8;
        changeObject = true;
    }

    if (changeObject) {

        if (selectedObject != nullptr) selectedObject->opacity = 1;
        selectedObject = objects->getById(selectedObjectId);
        selectedObject->opacity = 0.5;

        ui->updateWindowTransform(selectedObject);

    }

    if (state->keyJustDown(SDLK_RETURN)) {

        if (selectedObject != nullptr) selectedObject->opacity = 1;
        selectedObject = nullptr;

        ui->destroyWindowTransform();

    }

    // This rotates the selected object when pressing keys j,k,l
    if (state->keyIsDown(SDLK_j))
        if (selectedObject != nullptr) selectedObject->rotateSelf(1, 0, 0);

    if (state->keyIsDown(SDLK_k))
        if (selectedObject != nullptr) selectedObject->rotateSelf(0, 1, 0);

    if (state->keyIsDown(SDLK_l))
        if (selectedObject != nullptr) selectedObject->rotateSelf(0, 0, 1);

    // This moves the selected object along the y-axis when pressing keys o,p
    if (state->keyIsDown(SDLK_o))
        if (selectedObject != nullptr) selectedObject->move(0, 0.5, 0);

    if (state->keyIsDown(SDLK_p))
        if (selectedObject != nullptr) selectedObject->move(0, -0.5, 0);

    // This moves the selected object along the x-axis when pressing keys o,p
    if (state->keyIsDown(SDLK_u))
        if (selectedObject != nullptr) selectedObject->move(0.5, 0, 0);

    if (state->keyIsDown(SDLK_i))
        if (selectedObject != nullptr) selectedObject->move(-0.5, 0, 0);

    // This moves the selected object along the z-axis when pressing keys o,p
    if (state->keyIsDown(SDLK_t))
        if (selectedObject != nullptr) selectedObject->move(0, 0, 0.5);

    if (state->keyIsDown(SDLK_y))
        if (selectedObject != nullptr) selectedObject->move(0, 0, -0.5);

    // Toggles gravity
    if (state->keyJustDown(SDLK_g)) {
        
        if (gravity) {
            objects->setGravityAll(0.0);
            gravity = false;
        }

        else {
            objects->setGravityAll(-30);
            gravity = true;
        }

    }

    // Gives all the objects some vertical velocity
    if (state->keyJustDown(SDLK_z))
        objects->setVelocityAll(0, 25, 0);

    // Makes a new Transform window for the selected object
    // if (state->keyJustDown(SDLK_0))
    //     ui->createWindowTransform(selectedObject);

    // Give the state to the UI to handle
    ui->doInput(state);

}

void init() {

    // Log stuff
    logInit("log.txt");
    logClear();
    logWrite("Starting...", true);

    // Start the gui window
    gui = new Gui(1200, 700);
    gui->init();

    // Init all the working objects
    state = new State();

    display = new Display(gui->windowWidth, gui->windowHeight);
    camera = new Camera();
    camera->setPreset(0);

    drawer = new Drawer(gui->windowWidth, gui->windowHeight);
    drawer->initFont();

    // Sets up the default meshes, and creates the objects to be drawn
    Mesh::initMeshes();


    // Create and populate the object set
    objects = new ObjectSet();
    Object* newObject;

    /*   Main Objects   */

    newObject = new Object();
    newObject->mesh = Mesh::cubeMesh->copy()->scale(15)->move(0, 0, 50)->setColor(Color::WHITE);
    objects->pushBack(newObject, 1);

    newObject = new Object();
    newObject->mesh = Mesh::cubeMesh->copy();
    newObject->scaleBy(5)->move(0, 20, 50)->setColor(Color::GREY);
    objects->pushBack(newObject, 2);

    newObject = new Object();
    newObject->mesh = Mesh::cubeMesh->copy();
    newObject->scaleBy(10, 5, 15)->move(30, 10, 40)->rotateSelf(10, 0, 0)->setColor(Color::BLUE);
    objects->pushBack(newObject, 3);

    newObject = new Object();
    newObject->mesh = Mesh::capsuleMesh->copy();
    newObject->scaleBy(15)->move(0, -20, 50)->setColor(Color::GREEN);
    objects->pushBack(newObject, 4);

    newObject = new Object();
    newObject->mesh = Mesh::sphereMesh->copy();
    newObject->scaleBy(15, 40, 15)->move(-30, 0, 50)->setColor(Color::BLUE);
    objects->pushBack(newObject, 5);

    newObject = new Object();
    newObject->mesh = Mesh::sphereMesh->copy();
    newObject->scaleBy(25)->move(-30, 0, 50)->setColor(Color::WHITE);
    objects->pushBack(newObject, 6);

    newObject = new Object();
    newObject->mesh = Mesh::cubeMesh->copy();
    newObject->scaleBy(10)->move(0, 10, 50)->setColor(Color::BLUE);
    objects->pushBack(newObject, 7);


    /*   Floor   */

    newObject = new Object();
    newObject->mesh = Mesh::cubeMesh->copy();
    newObject->scaleBy(100, 1, 100)->move(0, -50, 50)->setColor(Color::DARKGREY);
    objects->pushBack(newObject, 8);


    gravity = false;
    drawNormals = false;

    selectedObjectId = 1;

    ui = new UI();

}

int main(int argc, char* argv[]) {

    // Starts up everything needed for the main loop
    init();

    // Declarations for the main loop
    SDL_Event event;
    bool leave = false;


    // Main loop
    while (!leave) {

        // Update mouse position
        SDL_GetMouseState(   &(state->mouse->posX),   &(state->mouse->posY)   );
        
        // Handle SDL events
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) { leave = true; std::cout<<"closed"; }
            else state->addEvent(&event);
        }

        // Does all the user input handling
        handleInput(state, camera);

        // Handle the physics for the frame
        objects->doAllPhysics(state->time->dt);

        // Prep the pixel and depth buffers
        gui->getBuffer();
        drawer->buffer = gui->buffer;
        drawer->resetDepthBuffer();
        drawer->drawSky(camera, display); // This acts as a pixel buffer reset since it draws to every pixel

        // Draw all the objects
        if (drawNormals) objects->drawAllWithNormals(drawer, camera, display);
        else objects->drawAll(drawer, camera, display);
        
        // Draw the UI
        drawer->drawFps(state, display);
        ui->draw(drawer);
        drawer->drawCrosshair(display);

        // Update the GUI
        gui->flip();

        // Tell State that the frame is over
        state->nextFrame();

    }

    delete state;
    delete camera;
    delete display;
    delete drawer;

    // Destroy the window and quit SDL
    gui->exit();
    delete gui;

    return 0;

}
