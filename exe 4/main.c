#define _CRT_SECURE_NO_WARNINGS
/*#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>*/ //uncomment this block to check for heap memory allocation leaks.
// Read https://docs.microsoft.com/en-us/visualstudio/debugger/finding-memory-leaks-using-the-crt-library?view=vs-2019
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct StudentCourseGrade
{
	char courseName[35];
	int grade;
}StudentCourseGrade;

typedef struct Student
{
	char name[35];
	StudentCourseGrade* grades; //dynamic array of courses
	int numberOfCourses;
}Student;


//Part A
void countStudentsAndCourses(const char* fileName, int** coursesPerStudent, int* numberOfStudents);
int countPipes(const char* lineBuffer, int maxCount);
char*** makeStudentArrayFromFile(const char* fileName, int** coursesPerStudent, int* numberOfStudents);
void printStudentArray(const char* const* const* students, const int* coursesPerStudent, int numberOfStudents);
void factorGivenCourse(char** const* students, const int* coursesPerStudent, int numberOfStudents, const char* courseName, int factor);
void studentsToFile(char*** students, int* coursesPerStudent, int numberOfStudents);

//Part B
Student* transformStudentArray(char*** students, const int* coursesPerStudent, int numberOfStudents);
void writeToBinFile(const char* fileName, Student* students, int numberOfStudents);
Student* readFromBinFile(const char* fileName);

//my functions
char* getDynStr(char* str);
void getStudent(FILE* f, char** student);
void freeStudents(char*** students, int* coursesPerStudent, int numberOfStudents);

int main()
{
	//Part A
	int* coursesPerStudent = NULL;
	int numberOfStudents = 0;
	char*** students = makeStudentArrayFromFile("studentList.txt", &coursesPerStudent, &numberOfStudents);
	factorGivenCourse(students, coursesPerStudent, numberOfStudents, "Advanced Topics in C", +5);
	printStudentArray(students, coursesPerStudent, numberOfStudents);
	//studentsToFile(students, coursesPerStudent, numberOfStudents); //this frees all memory. Part B fails if this line runs. uncomment for testing (and comment out Part B)


	////Part B
	Student* transformedStudents = transformStudentArray(students, coursesPerStudent, numberOfStudents);
	writeToBinFile("students.bin", transformedStudents, numberOfStudents);
	Student* testReadStudents = readFromBinFile("students.bin");

	//add code to free all arrays of struct Student
	studentsToFile(students, coursesPerStudent, numberOfStudents);
	for (int i = 0; i < numberOfStudents; i++)
	{
		free(transformedStudents[i].grades);//free grades*
		free(testReadStudents[i].grades);//free grades*
	}

	free(transformedStudents); //free students*
	free(testReadStudents); //free students*


	printf("%d\n", _CrtDumpMemoryLeaks()); //uncomment this block to check for heap memory allocation leaks.
	// Read https://docs.microsoft.com/en-us/visualstudio/debugger/finding-memory-leaks-using-the-crt-library?view=vs-2019

	return 0;
}

void countStudentsAndCourses(const char* fileName, int** coursesPerStudent, int* numberOfStudents)
{
	FILE* f = fopen(fileName, "r");
	int* arr = NULL;
	int size = 0;
	if (f == NULL) {
		printf("error opening file\n");
		return;
	}
	char str[1024];
	while (fgets(str, 1024, f) != NULL) {
		int pipeCount = countPipes(str, 1024);
		arr = (int*)realloc(arr, sizeof(int) * (size + 1));
		if (!arr)
		{
			printf("Allocation error\n");
			fclose(f);
			exit(1);
		}
		arr[size] = pipeCount;
		size++;
	}
	fclose(f);
	*coursesPerStudent = arr;
	*numberOfStudents = size;
}

int countPipes(const char* lineBuffer, int maxCount)
{
	if (lineBuffer == NULL) {
		return -1;
	}
	if (maxCount <= 0) {
		return 0;
	}
	int index = 0, counter = 0;
	while (lineBuffer[index] != '\0' && index < maxCount) {
		if (lineBuffer[index] == '|') {
			counter++;
		}
		index++;

	}
	return counter;
}

char*** makeStudentArrayFromFile(const char* fileName, int** coursesPerStudent, int* numberOfStudents)
{

	countStudentsAndCourses(fileName, coursesPerStudent, numberOfStudents);
	FILE* f = fopen(fileName, "r");
	if (f == NULL) {
		printf("error opening file\n");
		return NULL;
	}
	char*** students = NULL;
	students = (char***)malloc(sizeof(char**) * (*numberOfStudents));
	if (students == NULL) {
		fclose(f);
		printf("Allocation error\n");
		exit(1);
	}
	for (int i = 0; i < *numberOfStudents; i++) {
		int size = (1 + 2 * (*coursesPerStudent)[i]);
		students[i] = (char**)malloc(sizeof(char*) * (size));
		if (students[i] == NULL) {
			fclose(f);
			printf("Allocation error\n");
			exit(1);
		}
		getStudent(f, students[i]);
	}

	fclose(f);


	return students;
}






void factorGivenCourse(char** const* students, const int* coursesPerStudent, int numberOfStudents, const char* courseName, int factor)
{
	if (factor > 20 || factor < -20) {
		return;
	}
	for (int i = 0; i < numberOfStudents; i++) {
		char** student = students[i];
		int coursesSize = coursesPerStudent[i];
		for (int j = 0; j < coursesSize; j++) {
			if (strcmp(courseName, student[1 + j * 2]) == 0) {
				int grade = atoi(student[2 + j * 2]);
				grade += factor;
				if (grade > 100) {
					grade = 100;
				}
				else if (grade < 0) {
					grade = 0;
				}
				char gradeStr[4];
				_itoa(grade, gradeStr, 10);
				free(student[2 + j * 2]);
				student[2 + j * 2] = getDynStr(gradeStr);
			}
		}
	}
}

void printStudentArray(const char* const* const* students, const int* coursesPerStudent, int numberOfStudents)
{
	for (int i = 0; i < numberOfStudents; i++)
	{
		printf("name: %s\n*********\n", students[i][0]);
		for (int j = 1; j <= 2 * coursesPerStudent[i]; j += 2)
		{
			printf("course: %s\n", students[i][j]);
			printf("grade: %s\n", students[i][j + 1]);
		}
		printf("\n");
	}
}

void studentsToFile(char*** students, int* coursesPerStudent, int numberOfStudents)
{
	FILE* f = fopen("studentList.txt", "w");
	const char TOKEN1 = '|';
	const char TOKEN2 = ',';
	if (f == NULL) {
		printf("error opening file\n");
		return;
	}

	for (int i = 0; i < numberOfStudents; i++)
	{
		fprintf(f, "%s", students[i][0]);
		fprintf(f, "%c", TOKEN1);
		for (int j = 1; j <= 2 * coursesPerStudent[i]; j += 2)
		{
			fprintf(f, "%s%c%s", students[i][j], TOKEN2, students[i][j + 1]);
			if (j + 1 < 2 * coursesPerStudent[i])
				fprintf(f, "%c", TOKEN1);
		}
	}
	freeStudents(students, coursesPerStudent, numberOfStudents);
	fclose(f);
}

void writeToBinFile(const char* fileName, Student* students, int numberOfStudents)
{
	FILE* f = fopen(fileName, "wb");
	if (!f) {
		printf("file error");
		exit(1);
	}
	fwrite(&numberOfStudents, sizeof(int), 1, f);
	for (int i = 0; i < numberOfStudents; i++) {
		fwrite(students[i].name, sizeof(char), 35, f);
		fwrite(&students[i].numberOfCourses, sizeof(int), 1, f);
		fwrite(students[i].grades, sizeof(StudentCourseGrade), students[i].numberOfCourses, f);

	}
	fclose(f);
}

Student* readFromBinFile(const char* fileName)
{
	FILE* f = fopen(fileName, "rb");
	if (!f) {
		printf("file error");
		exit(1);
	}
	int numberOfStudents, numberOfCourses;
	char name[35];
	fread(&numberOfStudents, sizeof(int), 1, f);
	Student* arr = (Student*)malloc(sizeof(Student) * numberOfStudents);
	if (!arr) {
		printf("error");
		exit(1);
	}
	for (int i = 0; i < numberOfStudents; i++) {
		fread(name, sizeof(char), 35, f);
		strcpy(arr[i].name, name);
		fread(&numberOfCourses, sizeof(int), 1, f);
		arr[i].numberOfCourses = numberOfCourses;
		StudentCourseGrade* grades = (StudentCourseGrade*)malloc(sizeof(StudentCourseGrade) * numberOfCourses);
		if (!grades) {
			printf("error");
			exit(1);
		}
		fread(grades, sizeof(StudentCourseGrade), numberOfCourses, f);
		arr[i].grades = grades;
	}
	fclose(f);
	return arr;
}

Student* transformStudentArray(char*** students, const int* coursesPerStudent, int numberOfStudents)
{
	Student* arr = (Student*)malloc(sizeof(Student) * numberOfStudents);
	if (!arr) {
		printf("Allocation error\n");
		exit(1);
	}


	for (int i = 0; i < numberOfStudents; i++)
	{
		strcpy(arr[i].name, students[i][0]);
		arr[i].numberOfCourses = coursesPerStudent[i];
		arr[i].grades = (StudentCourseGrade*)malloc(sizeof(StudentCourseGrade) * arr[i].numberOfCourses);
		if (!arr[i].grades)
		{
			printf("Allocation error\n");
			exit(1);
		}
		for (int j = 1, k = 0; j <= 2 * coursesPerStudent[i]; j += 2, k++)
		{
			strcpy(arr[i].grades[k].courseName, students[i][j]);
			arr[i].grades[k].grade = atoi(students[i][j + 1]);
		}

	}

	return arr;
}

char* getDynStr(char* str)
{
	char* theStr;
	theStr = (char*)malloc((strlen(str) + 1) * sizeof(char));
	if (!theStr)
		return NULL;

	strcpy(theStr, str);
	return theStr;
}

void getStudent(FILE* f, char** student) {
	char str[1024];
	if (fgets(str, 1024, f) == NULL) {
		fclose(f);
		exit(1);
	}
	const char* TOKEN1 = "|";
	const char* TOKEN2 = ",";
	int isFirstTime = 1;
	int strIndex = 0;
	char* word = strtok(str, TOKEN1); //split line into sub string splited by TOKEN1 i.e "Avi" "Linear Algebra,90"
	while (word != NULL) {
		if (isFirstTime == 1) {
			char* studentName = getDynStr(word);
			if (studentName == NULL) {
				printf("Allocation error\n");
				fclose(f);
				exit(1);
			}
			student[strIndex] = studentName;
			strIndex++;
			isFirstTime = 0;
		}
		else {
			int comaIndex = strcspn(word, TOKEN2); //Find index of TOKEN2 i.e "Good,90" index = 4 (case TOKEN2 doesn't exist = -1)
			word[comaIndex] = '\0'; //Make course into string i.e "Good,90" => "Good"
			char* courseName = getDynStr(word);
			if (courseName == NULL) {
				printf("Allocation error\n");
				fclose(f);
				exit(1);
			}
			student[strIndex] = courseName;
			strIndex++;
			word += comaIndex + 1; //Index of TOKEN2 is known, moving pointer to grade i.e word="Good\090" => word = "90"
			char* courseGrade = getDynStr(word);
			if (courseGrade == NULL) {
				printf("Allocation error\n");
				fclose(f);
				exit(1);
			}
			student[strIndex] = courseGrade;
			strIndex++;
		}
		word = strtok(NULL, TOKEN1); //Find the next TOKEN1 and put it into word
	}
}

void freeStudents(char*** students, int* coursesPerStudent, int numberOfStudents)
{
	for (int i = 0; i < numberOfStudents; i++)
	{
		free(students[i][0]);
		for (int j = 1; j <= 2 * coursesPerStudent[i]; j += 2)
		{
			free(students[i][j]);
			free(students[i][j + 1]);
		}
		free(students[i]);
	}
	free(coursesPerStudent);
	free(students);
}
