#include "../include/ObjectSet.h"




/* -------------- */
/* --- Object --- */
/* -------------- */

// Constructors
Object::Object() {
    this->mesh = nullptr;
}

Object::Object(Mesh* mesh) {
    this->mesh = mesh;
}



/* ----------------------- */
/* --- ObjectSet::Node --- */
/* ----------------------- */

// Constructors
ObjectSet::Node::Node() {
    this->next = nullptr;
    this->last = nullptr;
    this->obj = nullptr;
    this->id = 0;
}

ObjectSet::Node::Node(Object* obj) {
    this->next = nullptr;
    this->last = nullptr;
    this->obj = obj;
    this->id = 0;
}

ObjectSet::Node::Node(int id) {
    this->next = nullptr;
    this->last = nullptr;
    this->obj = nullptr;
    this->id = id;
}

ObjectSet::Node::Node(Object* obj, int id) {
    this->next = nullptr;
    this->last = nullptr;
    this->obj = obj;
    this->id = id;
}

// Instance Functions
bool ObjectSet::Node::isFirst() {
    return (this->last == nullptr);
}

bool ObjectSet::Node::isLast() {
    return (this->next == nullptr);
}

void ObjectSet::Node::log() {

    if (this->last == nullptr)
        logWrite("null");
    else
        logWrite(this->last->id);

    logWrite(" <- ");
    logWrite(this->id);
    logWrite(" -> ");

    if (this->next == nullptr)
        logWrite("null", true);
    else
        logWrite(this->next->id, true);

    return;

}



/* ----------------- */
/* --- ObjectSet --- */
/* ----------------- */

// Constructor
ObjectSet::ObjectSet() {
    this->first = nullptr;
    this->last = nullptr;

    this->length = 0;
    this->nextFreeId = 0;
}

// Destructor
ObjectSet::~ObjectSet() {
    
    Node* currentRef = this->first;
    Node* nextRef = this->first->next;

    while (true) {

        delete currentRef;
        
        // Break case
        if (nextRef == nullptr) break;

        currentRef = nextRef;
        nextRef = currentRef->next;

    }

}

// Instance Functions
void ObjectSet::pushBack(Object* obj) {
    
    this->pushBack(obj, this->nextFreeId);

    this->nextFreeId++;

    return;

}

void ObjectSet::pushBack(Object* mesh, int id) {
    
    Node* newNode = new Node(mesh, id);

    this->length++;

    // Empty list
    if (this->first == nullptr && this->last == nullptr) {
        this->first = newNode;
        this->last = newNode;
        return;
    }

    // Length 1 list
    if (this->first == this->last) {
        this->last = newNode;
        this->first->next = newNode;
        newNode->last = this->first;
        return;
    }

    // Length >2 list
    this->last->next = newNode;
    newNode->last = this->last;
    this->last = newNode;

    return;

}

void ObjectSet::pushFront(Object* mesh) {
    
    this->pushFront(mesh, this->nextFreeId);

    this->nextFreeId++;

    return;

}

void ObjectSet::pushFront(Object* mesh, int id) {

    Node* newNode = new Node(mesh, id);

    this->length++;

    // Empty list
    if (this->first == nullptr && this->last == nullptr) {
        this->first = newNode;
        this->last = newNode;
        return;
    }

    // Length 1 list
    if (this->first == this->last) {
        this->first = newNode;
        this->last->last = newNode;
        newNode->next = this->last;
        return;
    }

    // Length >2 list
    this->first->last = newNode;
    newNode->next = this->first;
    this->first = newNode;

    return;
    
}

Object* ObjectSet::popBack() {

    Node* ret;
    Object* obj;

    // Empty list
    if (this->first == nullptr && this->last == nullptr) {
        return nullptr;
    }

    this->length--;

    // Length 1 list
    if (this->first == this->last) {

        ret = this->last;
        this->last = nullptr;
        this->first = nullptr;

        obj = ret->obj;
        delete ret;

        return obj;

    }

    // Length >2 list
    ret = this->last;
    this->last = ret->last;
    this->last->next = nullptr;

    obj = ret->obj;
    delete ret;

    return obj;

}

Object* ObjectSet::popFront() {

    Node* ret;
    Object* obj;

    // Empty list
    if (this->first == nullptr && this->last == nullptr) {
        return nullptr;
    }

    this->length--;

    // Length 1 list
    if (this->first == this->last) {

        ret = this->first;
        this->last = nullptr;
        this->first = nullptr;

        obj = ret->obj;
        delete ret;

        return obj;

    }

    // Length >2 list
    ret = this->first;
    this->first = ret->next;
    this->first->last = nullptr;

    obj = ret->obj;
    delete ret;

    return obj;
    
}

Object* ObjectSet::popById(int id) {

    Node* current = this->first;

    while (true) {

        if (current == nullptr) return nullptr;

        if (current->id == id) {

            current->last->next = current->next;
            current->next->last = current->last;

            Object* ret = current->obj;

            delete current;
            this->length--;

            return ret;

        }

        current = current->next;

    }

}

Object* ObjectSet::getById(int id) {

    Node* current = this->first;

    while (true) {

        if (current == nullptr) return nullptr;

        if (current->id == id)
            return current->obj;

        current = current->next;

    }

}

void ObjectSet::iterStart(int index) {

    this->iterCurrent = this->first;

    for (int i = 0; i < index; i++) {

        if (iterCurrent == nullptr) {
            logWrite("Tried to start at iter at too high of an index! (ObjectSet->iterStart(int))");
            return;
        }

        this->iterCurrent = this->iterCurrent->next;

    }

}

Object* ObjectSet::iterGetObj() {

    if (this->iterCurrent == nullptr) return nullptr;

    return this->iterCurrent->obj;

}

int ObjectSet::iterGetId() {

    if (this->iterCurrent == nullptr) return -1;

    return this->iterCurrent->id;

}

void ObjectSet::iterNext() {

    if (this->iterCurrent == nullptr) return;

    this->iterCurrent = this->iterCurrent->next;
    return;

}

void ObjectSet::iterLast() {

    if (this->iterCurrent == nullptr) return;

    this->iterCurrent = this->iterCurrent->last;
    return;

}

bool ObjectSet::iterIsDone() {

    return (this->iterCurrent == nullptr);

}

void ObjectSet::swap(Node* node1, Node* node2) {

    // Different handling for when the two nodes are adjacent
    if (node1->next == node2) {
        
        // Update adjacent node pointers
        if (node1->last != nullptr) node1->last->next = node2;
        else this->first = node2;
        if (node2->next != nullptr) node2->next->last = node1;
        else this->last = node1;

        // Update affected node pointers
        node1->next = node2->next;
        node2->next = node1;

        node2->last = node1->last;
        node1->last = node2;

        return;

    }

    if (node2->next == node2) {
        
        // Update adjacent node pointers
        if (node2->last != nullptr) node2->last->next = node1;
        else this->first = node1;
        if (node1->next != nullptr) node1->next->last = node2;
        else this->last = node2;

        // Update affected node pointers
        node2->next = node1->next;
        node1->next = node2;

        node1->last = node2->last;
        node2->last = node1;

        return;

    }

    // Get adjacent nodes
    Node* node1Last = node1->last;
    Node* node1Next = node1->next;

    Node* node2Last = node2->last;
    Node* node2Next = node2->next;

    // Update adjacent node pointers
    if (node1Last != nullptr) node1Last->next = node2;
    if (node1Next != nullptr) node1Next->last = node2;

    if (node2Last != nullptr) node2Last->next = node1;
    if (node2Next != nullptr) node2Next->last = node1;

    // Update affected node pointers
    node1->next = node2Next;
    node1->last = node2Last;

    node2->last = node1Last;
    node2->next = node1Next;

    return;

}

void ObjectSet::projectAll(Camera* camera, Display* display) {

    Node* current = this->first;

    for (int i = 0; i < this->length; i++) {

        if (current == nullptr) {
            logWrite("Null Iteration!", true);
            break;
        }

        camera->project(current->obj->mesh);
        display->toDisplayPos(current->obj->mesh);

        current = current->next;

    }

}

void ObjectSet::sortByDistance(Camera* camera) {

    /*
        Bubble sort approach using ObjectSet::swap(Node*, Node*)
    */

    for (int i = 0; i < this->length - 1; i++) {

        Node* current = this->first;
        Node* next = current->next;

        for (int j = 0; j < (this->length - (i + 1)); j++) {

            if (current == nullptr || next == nullptr) {
                logWrite("nullptr issue!", true);
                break;
            }

            Vec3* currentMeshPos = current->obj->mesh->getCenter();
            Vec3* nextMeshPos = next->obj->mesh->getCenter();
            Vec3* camPos = camera->pos;
            double currentDist = currentMeshPos->distanceTo(camPos);
            double nextDist = nextMeshPos->distanceTo(camPos);

            if (currentDist < nextDist)
                this->swap(current, next);

            else 
                current = next;
            
            next = current->next;

        }

    }

}

void ObjectSet::drawAll(Drawer* drawer, Camera* camera, Display* display) {

    // Address error cases, but dont kill the process yet in case its not fatal
    if (drawer == nullptr) {
        logWrite("Called ObjectSet->drawAll(Drawer*, Camera*) with 'drawer' as a null pointer!", true);
        return;
    }

    if (camera == nullptr) {
        logWrite("Called ObjectSet->drawAll(Drawer*, Camera*) with 'camera' as a null pointer!", true);
        return;
    }

    // Pre Set up
    this->projectAll(camera, display);
    this->sortByDistance(camera);

    // Main set up
    this->iterStart(0);
    Object* currentObject;
    Mesh* currentMesh;

    // Main loop
    while (!this->iterIsDone()) {

        currentObject = this->iterGetObj();
        currentMesh = currentObject->mesh;

        for (int i = 0; i < currentMesh->triCount; i++) {
        
            // Skip if tri cant be seen by cam on the outfacing side
            if (!camera->canSee(currentMesh->tris[i])) continue;

            // Find a shade based on the lighting vec
            double lightAngle = currentMesh->tris[i]->normal->getAngle(camera->lightingVec);
            double lightFactor = lightAngle / 180;

            Uint32 shade = currentMesh->color;
            shade = Color::setBrightness(shade, lightFactor);

            // Draw the tri    
            drawer->drawTriangle(shade, currentMesh->projectedTris[i]);

        }

        this->iterNext();

    }

}

void ObjectSet::drawAllWithNormals(Drawer* drawer, Camera* camera, Display* display) {

    // Address error cases, but dont kill the process yet in case its not fatal
    if (drawer == nullptr) {
        logWrite("Called ObjectSet->drawAllWithNormals(Drawer*, Camera*, Display*) with 'drawer' as a null pointer!", true);
        return;
    }

    if (camera == nullptr) {
        logWrite("Called ObjectSet->drawAllWithNormals(Drawer*, Camera*, Display*) with 'camera' as a null pointer!", true);
        return;
    }

    if (display == nullptr) {
        logWrite("Called ObjectSet->drawAllWithNormals(Drawer*, Camera*, Display*) with 'display' as a null pointer!", true);
        return;
    }

    // Pre Set up
    this->projectAll(camera, display);
    this->sortByDistance(camera);

    // Main set up
    this->iterStart(0);
    Object* currentObject;
    Mesh* currentMesh;
    Vec2* vecStart = new Vec2(); 
    Vec2* vecEnd = new Vec2();

    // Main loop
    while (!this->iterIsDone()) {

        currentObject = this->iterGetObj();
        currentMesh = currentObject->mesh;

        for (int i = 0; i < currentMesh->triCount; i++) {
        
            // Skip if tri cant be seen by cam on the outfacing side
            if (!camera->canSee(currentMesh->tris[i])) continue;

            // Find a shade based on the lighting vec
            double lightAngle = currentMesh->tris[i]->normal->getAngle(camera->lightingVec);
            double lightFactor = lightAngle / 180;

            Uint32 shade = currentMesh->color;
            shade = Color::setBrightness(shade, lightFactor);

            // Draw the tri    
            drawer->drawTriangle(shade, currentMesh->projectedTris[i]);

        }

        // Draw normals
        for (int i = 0; i < currentMesh->triCount; i++) {

            // Skip if tri cant be seen face on by cam
            if (!camera->canSee(currentMesh->tris[i])) continue;

            Vec3* triCenter;
            Vec3* normalOffset;
            Vec3* triNormal;

            // Get projected coords for normal start and end
            triCenter = currentMesh->tris[i]->getCenter();
            normalOffset = currentMesh->tris[i]->normal->copy()->normalise(1);
            triNormal = triCenter->copy()->add(normalOffset);

            camera->project(triCenter, vecStart);
            camera->project(triNormal, vecEnd);
            display->toDisplayPos(vecStart);
            display->toDisplayPos(vecEnd);

            drawer->drawLine(Color::RED, vecStart, vecEnd);

            delete triCenter;
            delete normalOffset;
            delete triNormal;

        }

        this->iterNext();

    }

    delete vecStart; delete vecEnd;

}

void ObjectSet::log() {

    logWrite("ObjectSet( length = ");
    logWrite(this->length, true);

    Node* current = this->first;

    while (true) {

        if (current == nullptr) break;

        logWrite("  ");
        current->log();

        current = current->next;

    }

    logWrite(")", true);

    return;

}
