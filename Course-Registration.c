#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_NAME 100

struct Course {
    int id;
    char name[MAX_NAME];
    struct Course *left, *right;
};

struct CourseNode {
    int courseId;
    struct CourseNode *next;
};

struct Student {
    int id;
    char name[MAX_NAME];
    struct CourseNode *courses;
    struct Student *next;
};

struct Request {
    int studentId;
    int courseId;
    struct Request *next;
};

struct UndoAction {
    int studentId;
    int courseId;
    struct UndoAction *next;
};


struct Student *studentHead = NULL;
struct Course *courseRoot = NULL;
struct Request *queueFront = NULL, *queueRear = NULL;
struct UndoAction *undoTop = NULL;

void pushUndo(int studentId, int courseId) {
    struct UndoAction *newAction = (struct UndoAction*)malloc(sizeof(struct UndoAction));
    newAction->studentId = studentId;
    newAction->courseId = courseId;
    newAction->next = undoTop;
    undoTop = newAction;
}

int popUndo(int *studentId, int *courseId) {
    if (!undoTop)
      return 0;

    struct UndoAction *temp = undoTop;
    *studentId = temp->studentId;
    *courseId = temp->courseId;
    undoTop = undoTop->next;
    free(temp);
    return 1;
}


void enqueue(int studentId, int courseId) {
    struct Request* newReq = (struct Request*)malloc(sizeof(struct Request));
    newReq->studentId = studentId;
    newReq->courseId = courseId;
    newReq->next = NULL;

    if (!queueRear) {
        queueFront = queueRear = newReq;
    } else {
        queueRear->next = newReq;
        queueRear = newReq;
    }
}

int dequeue(int *studentId, int *courseId) {
    if (!queueFront)
        return 0;
    struct Request *temp = queueFront;
    *studentId = temp->studentId;
    *courseId = temp->courseId;
    queueFront = queueFront->next;

    if (!queueFront) queueRear = NULL;
    free(temp);
    return 1;
}

struct Course* insertCourse(struct Course* root, int id, char name[]) {
    if (!root) {
        struct Course *newCourse = (struct Course*)malloc(sizeof(struct Course));
        newCourse->id = id;
        strcpy(newCourse->name, name);
        newCourse->left = NULL;
        newCourse->right = NULL;
        return newCourse;
    }
    if (id < root->id)
        root->left = insertCourse(root->left, id, name);

    else if (id > root->id)
    root->right = insertCourse(root->right, id, name);

    return root;
}

struct Course* searchCourse(struct Course* root, int id) {
    if (!root || root->id == id)
        return root;

    if (id < root->id)
        return searchCourse(root->left, id);
    else
        {
            return searchCourse(root->right, id);
        }
}

struct Student* findStudent(int id) {
    struct Student *curr = studentHead;
    while (curr) {
        if (curr->id == id) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

void addStudent(int id, char name[]) {
    if (findStudent(id)) {
        printf("Student already exists.\n");
        return;
    }
    struct Student* newStudent = (struct Student*)malloc(sizeof(struct Student));
    newStudent->id = id;
    strcpy(newStudent->name, name);
    newStudent->courses = NULL;
    newStudent->next = studentHead;
    studentHead = newStudent;
    printf("Student %s added.\n", name);
}

int isAlreadyRegistered(struct Student *student, int courseId) {
    struct CourseNode *curr = student->courses;
    while (curr) {
        if (curr->courseId == courseId)
            return 1;
        curr = curr->next;
    }
    return 0;
}

void registerCourse(struct Student *student, int courseId) {
    if (isAlreadyRegistered(student, courseId)) {
        printf("Student %s is already registered for course ID %d.\n", student->name, courseId);
        return;
    }
    struct CourseNode* newCourse = (struct CourseNode*)malloc(sizeof(struct CourseNode));
    newCourse->courseId = courseId;
    newCourse->next = student->courses;
    student->courses = newCourse;
}


void processRequests() {
    int studentId, courseId;
    while (dequeue(&studentId, &courseId)) {
        struct Student *student = findStudent(studentId);
        struct Course *course = searchCourse(courseRoot, courseId);
        if (student && course) {
            if (!isAlreadyRegistered(student, courseId)) {
                registerCourse(student, courseId);
                pushUndo(studentId, courseId);
                printf("Registered %s to %s\n", student->name, course->name);
            } else {
                printf("%s is already registered for %s\n", student->name, course->name);
            }
        } else {
            printf("Invalid request: Student or Course not found.\n");
        }
    }
}

void undoLastAction() {
    int studentId, courseId;
    if (popUndo(&studentId, &courseId)) {
        struct Student *student = findStudent(studentId);
        if (student) {
            struct CourseNode *curr = student->courses, *prev = NULL;
            while (curr && curr->courseId != courseId) {
                prev = curr;
                curr = curr->next;
            }
            if (curr) {
                if (prev) prev->next = curr->next;
                else student->courses = curr->next;
                free(curr);
                printf("Undo: Removed course %d from student %s\n", courseId, student->name);
            }
        }
    } else {
        printf("Nothing to undo.\n");
    }
}

void displayStudents() {
    struct Student *s = studentHead;
    while (s) {
        printf("Student ID: %d, Name: %s, Courses: ", s->id, s->name);
        struct CourseNode *c = s->courses;
        if (!c)
            printf("None");
        while (c) {
            struct Course *course = searchCourse(courseRoot, c->courseId);
            if (course) printf("[%d-%s] ", course->id, course->name);
            c = c->next;
        }
        printf("\n");
        s = s->next;
    }
}


int main() {
    int choice, sid, cid;
    char name[MAX_NAME];

    while (1) {
        printf("\n--- University Course Registration System ---\n");
        printf("1. Add Student\n");
        printf("2. Add Course(s)\n");
        printf("3. Enqueue Multiple Registration Requests\n");
        printf("4. Process Requests\n");
        printf("5. Undo Last Action\n");
        printf("6. Display Students\n");
        printf("7. Find Student\n");
        printf("8. Search Course\n");
        printf("9. Exit\nChoice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter student ID and name: ");
                scanf("%d %s", &sid, name);
                addStudent(sid, name);
                break;
            case 2: {
                int n;
                printf("How many courses do you want to add? ");
                scanf("%d", &n);
                for (int i = 0; i < n; i++) {
                    printf("Enter course ID and name: ");
                    scanf("%d %s", &cid, name);
                    courseRoot = insertCourse(courseRoot, cid, name);
                    printf("Course %s added.\n", name);
                }
                break;
            }
            case 3: {
                int n;
                printf("Enter student ID: ");
                scanf("%d", &sid);
                printf("How many courses to register? ");
                scanf("%d", &n);
                for (int i = 0; i < n; i++) {
                    printf("Enter course ID: ");
                    scanf("%d", &cid);
                    enqueue(sid, cid);
                }
                break;
            }
            case 4:
                processRequests();
                break;
            case 5:
                undoLastAction();
                break;
            case 6:
                displayStudents();
                break;
            case 7:
                printf("Enter student ID : ");
                scanf("%d", &sid);
                {
                struct Student *s = findStudent(sid);
                if (s)
                printf("Student found: ID=%d, Name=%s\n", s->id, s->name);
                else
                printf("Student with ID %d not found.\n", sid);
                }
                break;
            case 8:
                printf("Enter course ID to search: ");
                scanf("%d", &cid);
                {
                struct Course *course = searchCourse(courseRoot, cid);
                if (course)
                    printf("Course found: [%d - %s]\n", course->id, course->name);
                else
                    printf("Course with ID %d not found.\n", cid);
                }
                break;
            case 9:
                exit(0);
            default:
                printf("Invalid choice.\n");
        }
    }
    return 0;
}
