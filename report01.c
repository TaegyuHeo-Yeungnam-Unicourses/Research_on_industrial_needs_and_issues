#include <stdio.h>

void get_school_position(int school_index, int* posX, int* posY);
void get_student_position(int student_index, int* posX, int* posY);
void set_student_school(int student_index, int school_index);

#define __INT_MAX__ 2147483647
#define __INT_MIN__ -2147483648
#define __FLOAT_MAX__ 3.402823466e+38F
#define __FLOAT_MIN__ -3.402823466e+38F

typedef struct s_point
{
	int x;
	int y;

	float x_f;
	float y_f;
	float dist_from_circumcenter; /* for sorting optimization */
}	t_point;

typedef struct s_student
{
	int index;
	t_point pos;
}	t_student;

static const char *error_messages[] = {
	"An unknown error occurred",
	"INT_MIN cannot return an absolute value",
	"power_f function do not support negative powers",
	"failure get a sqrt val of x",
	"x is too big to get sqrt val",
	"x is too small to get sqrt val"
};

const static char* get_error_message(int code) {
    if (code < 2) 
	{
        return error_messages[0];
    }
    return error_messages[code];
}

//do not check overflow
static float power_f(float x, int r)
{
	float result;
	int i;

	if (r < 0)
		printf("ERROR : %s\n", get_error_message(2));
	for (i = 0, result = 1; i < r; i++)
	{
		result *= x;
	}
	return (result);
}

//소수점 3자리까지만 비교하여 구함.
static float sqrt(float x)
{
	float i;
	float pow_val;
	float tmp1;
	float tmp2;

	if (x < 1.0f)
		printf("ERROR: %S\n", get_error_message(5));
	if (x > __FLOAT_MAX__ / 1000)
		printf("ERROR: %s\n", get_error_message(4));
	tmp2 = x;
	tmp2 *= power_f(10, 3);
	//0.001씩 더하는 건 매우 효율 나쁜 짓이긴 함.
	//차후 다소의 수정이 필요.
	for (i=1.0f, pow_val=1.0f; i+=0.001f; pow_val < x)
	{
		pow_val = power_f(i, 2);
		tmp1 = pow_val;
		tmp1 *= power_f(10, 3);
		if ((int)tmp1 == (int)tmp2)
			return (i);
	}
	printf("ERROR: %s\n", get_error_message(3));
	return(-1.0f);
}

static int abs(int x)
{
	if (x == __INT_MIN__)
		printf("ERROR : %s\n", get_error_message(1));
	if (x < 0)
		return -x;
	else
		return x;
}

static float get_dist(t_point a, t_point b)
{
	float dist;
	int dist_x;
	int dist_y;
	float dist_x_squared;
	float dist_y_squared;

	dist_x = abs(b.x - a.x);
	dist_y = abs(b.y - a.y);
	dist_x_squared = power_f((float)dist_x, 2);
	dist_y_squared = power_f((float)dist_y, 2);
	dist = sqrt(dist_x_squared + dist_y_squared);
	return (dist);
}

static void set_circumcenter_of_schools(t_point *circumcenter, t_point schools[3])
{
	/*
		Compute circumcenter of triangle (A,B,C).
		Uses standard determinant formula; if degenerate (collinear), fallback to centroid.
	*/
	float ax = schools[0].x_f;
	float ay = schools[0].y_f;
	float bx = schools[1].x_f;
	float by = schools[1].y_f;
	float cx = schools[2].x_f;
	float cy = schools[2].y_f;

	float a2 = ax * ax + ay * ay;
	float b2 = bx * bx + by * by;
	float c2 = cx * cx + cy * cy;

	float d = 2.0f * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
	/* very small determinant => nearly collinear */
	if (d < 0.000001f && d > -0.000001f)
	{
		circumcenter->x_f = (ax + bx + cx) / 3.0f;
		circumcenter->y_f = (ay + by + cy) / 3.0f;
	}
	else
	{
		circumcenter->x_f = (a2 * (by - cy) + b2 * (cy - ay) + c2 * (ay - by)) / d;
		circumcenter->y_f = (a2 * (cx - bx) + b2 * (ax - cx) + c2 * (bx - ax)) / d;
	}
	/* keep int fields consistent (best-effort rounding) */
	circumcenter->x = (int)(circumcenter->x_f);
	circumcenter->y = (int)(circumcenter->y_f);
}

static void sort_students_far_to_near_from_circumcenter(t_point *circumcenter, t_student students[10000])
{
	for (int i = 0; i < 10000; i++)
	{
		students[i].pos.dist_from_circumcenter = get_dist(*circumcenter, students[i].pos);
	}
	for (int i = 1; i < 10000; i++)
	{
		t_student key = students[i];
		int j = i - 1;
		while (j >= 0 && students[j].pos.dist_from_circumcenter < key.pos.dist_from_circumcenter)
		{
			students[j + 1] = students[j];
			j--;
		}
		students[j + 1] = key;
	}
}

static void allocate_students(t_point schools[3], t_student student)
{
	int best = 0;
	float best_d = get_dist(schools[0], student.pos);
	for (int i = 1; i < 3; i++)
	{
		float d = get_dist(schools[i], student.pos);
		if (d < best_d)
		{
			best_d = d;
			best = i;
		}
	}
	set_student_school(student.index, best);
}

static void init_school_position(t_point school[3])
{
	get_school_position(0, &school[0].x, &school[0].y);
	get_school_position(1, &school[1].x, &school[1].y);
	get_school_position(2, &school[2].x, &school[2].y);

	school[0].x_f = (float)school[0].x;
	school[0].y_f = (float)school[0].y;

	school[1].x_f = (float)school[1].x;
	school[1].y_f = (float)school[1].y;

	school[2].x_f = (float)school[2].x;
	school[2].y_f = (float)school[2].y;
}

void run_contest(void)
