/* gunhoflash

	[ 5*5 Sliding Puzzle ]
		University of Seoul, Department of Computer Science and Engineering
		2019-1 Computer Algorithm - Team 3

*/
#include <stdio.h>
#include <stdlib.h>
#define QUEUE_LENGTH_MAX 524288
#define HOLE 25
#define U 'U'
#define D 'D'
#define R 'R'
#define L 'L'

int score_coefficient;
int search_bound;

struct pstate
{
	bool visit;
	char type;           // U, D, R, L, or NULL
	int i;               // position of HOLE
	int j;               // position of HOLE
	int m[5][6];         // 1 ~ 24 and HOLE
	int different;
	int move;
	pstate* prev = NULL; // previous state
} typedef pstate;

// FOR TEST: check solution
void solution_check();

// print the solution
void print_solution(int attempt, char *solution_list, int solution_length)
{
	int i;
	printf("#%d %d", attempt, solution_length);
	for (i = 0; i < solution_length; i++)
		printf(" %c", solution_list[i]);
	printf("\n");
}

// update the solution
void update_solution(pstate* state, char *solution_list, int *solution_length)
{
	pstate* s = state;

	// not a short route
	if (*solution_length != -1 && s->move >= *solution_length) return;

	*solution_length = s->move;
	solution_list[*solution_length] = NULL;

	// save it as in-order
	while (s->prev != NULL)
	{
		solution_list[s->move - 1] = s->type;
		s = s->prev;
	}
}

// return state's solvability
bool isSolvable(pstate state)
{
	// refer to: https://www.cs.bham.ac.uk/~mdr/teaching/modules04/java2/TilesSolvability.html
	int i, j, n, inversion, total_inversion = 0;

	// if different is odd, state unsolvable
	if (state.different % 2 != 0) return false;

	// calculate inversion
	for (n = 1; n < 25; n++)
	{
		inversion = -1;
		for (i = 0; i < 5; i++)
			for (j = 0; j < 5; j++)
			{
				if (inversion == -1)
				{
					if (state.m[i][j] == n)
						inversion = 0;
				}
				else if (state.m[i][j] < n)
					inversion++;
			}
		total_inversion += inversion;
	}

	// if inversion is odd, state unsolvable
	return (total_inversion % 2 == 0);
}

// calculate different of the given state
void calculate_state(pstate *state)
{
	int i, j, m, t1, t2;
	state->different = 0;
	for (i = 0; i < 5; i++)
		for (j = 0; j < 5; j++)
		{
			m = state->m[i][j];
			if (m == HOLE)
			{
				state->different += 8 - i - j;
				state->i = i;
				state->j = j;
			}
			else
			{
				t1 = (m - 1) / 5 - i;
				t2 = (m - 1) % 5 - j;
				if (t1 < 0) t1 *= -1;
				if (t2 < 0) t2 *= -1;
				state->different += t1 + t2;
			}
		}
}

// find next states, set moves, and calculate
void add_new_state(pstate **states, int *states_length, pstate* prev, char type, int solution_length)
{
	int i, j, k, l;
	pstate* state;

	// get the position of HOLE
	i = prev->i;
	j = prev->j;

	// allocate memory
	while (states[*states_length] == NULL)
		states[*states_length] = (pstate*)malloc(sizeof(pstate));

	// initialize the state
	state = states[*states_length];
	state->visit = false;
	state->type = type;
	for (k = 0; k < 5; k++)
		for (l = 0; l < 5; l++)
			state->m[k][l] = prev->m[k][l];
	switch (type)
	{
		case U: state->m[i][j] = state->m[i - 1][j]; state->m[i - 1][j] = HOLE; break;
		case D: state->m[i][j] = state->m[i + 1][j]; state->m[i + 1][j] = HOLE; break;
		case L: state->m[i][j] = state->m[i][j - 1]; state->m[i][j - 1] = HOLE; break;
		case R: state->m[i][j] = state->m[i][j + 1]; state->m[i][j + 1] = HOLE; break;
	}
	for (k = 0; k < 5; k++)
	{
		state->m[k][5] =
			state->m[k][0] << 20 ^
			state->m[k][1] << 15 ^
			state->m[k][2] << 10 ^
			state->m[k][3] << 5 ^
			state->m[k][4];
	}
	state->move = prev->move + 1;
	calculate_state(state);
	state->prev = prev;

	// add only possibly-short state
	if (solution_length != -1)
		if (state->move + state->different / 2 >= solution_length) return;

	// add only new state
	k = *states_length;
	while (k--)
	{
		if (states[k]->different != state->different) continue;
		if (states[k]->m[2][5] != state->m[2][5]) continue;
		if (states[k]->m[3][5] != state->m[3][5]) continue;
		if (states[k]->m[1][5] != state->m[1][5]) continue;
		if (states[k]->m[4][5] != state->m[4][5]) continue;
		if (states[k]->m[0][5] == state->m[0][5]) return;
	}

	*states_length = *states_length + 1;
}

void set_next_state(pstate **states, int states_length, pstate **state, int solution_length)
{
	int i = states_length;
	int s1, s2;
	pstate *next_state = NULL;
	while (i--)
	{
		if (states[i]->visit) continue;
		// if (states[i] == *state) continue; // state is already visited
		if (solution_length != -1 && states[i]->move >= solution_length) continue;
		if (next_state == NULL) next_state = states[i];
		else
		{
			s1 = next_state->different * score_coefficient + (next_state->move << 6);
			s2 = states[i]->different * score_coefficient + (states[i]->move << 6);
			if (s1 > s2)
				next_state = states[i];
			else if (s1 == s2)
				if (next_state->move > states[i]->move)
					next_state = states[i];
			if (i < search_bound) break;
		}
	}
	*state = next_state;
}

void state_test(int attempt, pstate *first)
{
	int i, j;
	
	// solution
	char* solution_list = NULL;
	int solution_length = -1;

	// states
	pstate *state = first;
	pstate **states = NULL;
	int states_length = 1;

	// initialize global variables
	score_coefficient = 64;
	search_bound = 0;

	// allocate memory and initialize
	while (solution_list == NULL)
		solution_list = (char*)malloc(sizeof(char) * QUEUE_LENGTH_MAX);
	while (states == NULL)
		states = (pstate **)malloc(sizeof(pstate*) * QUEUE_LENGTH_MAX);
	for (i = 0; i < QUEUE_LENGTH_MAX; i++)
	{
		solution_list[i] = NULL;
		states[i] = NULL;
	}

	// set the first state
	state->type = NULL;
	state->move = 0;
	state->prev = NULL;
	calculate_state(state);
	states[0] = state;

	// check state solvability
	if (isSolvable(*state))
	{
		while (true)
		{
			// no more route
			if (state == NULL) break;

			state->visit = true;

			// if this state's score is 0, break;
			if (state->different == 0)
			{
				update_solution(state, solution_list, &solution_length);
				set_next_state(states, states_length, &state, solution_length);
				if (states_length > 72000) break;
				continue;
			}

			// no more searching
			if (states_length + 5 > QUEUE_LENGTH_MAX) break;

			// add next states
			if (solution_length == -1 || state->move + state->different / 2 < solution_length)
			{
				// set i and j as the position of HOLE
				i = state->i;
				j = state->j;

				if (i < 4 && state->type != U) add_new_state(states, &states_length, state, D, solution_length);
				if (j < 4 && state->type != L) add_new_state(states, &states_length, state, R, solution_length);
				if (i > 0 && state->type != D) add_new_state(states, &states_length, state, U, solution_length);
				if (j > 0 && state->type != R) add_new_state(states, &states_length, state, L, solution_length);
			}

			// tune
			if (states_length > 25000)
			{
				score_coefficient = 28 + (states_length >> 9);
				search_bound = states_length * 0.7;
				if (solution_length >= 0 && states_length > 72000) break;
			}
			
			// pick a state with lowest score
			set_next_state(states, states_length, &state, solution_length);
		}
	}

	print_solution(attempt, solution_list, solution_length);
}

void puzzleSolution()
{
	int i, j, k, t;
	pstate *states = NULL;

	// get the number of test case
	scanf("%d", &t);

	// execute solution check mode
	if (t <= 0)
	{
		solution_check();
		return;
	}

	// allocate memory
	while (states == NULL)
		states = (pstate*)malloc(sizeof(pstate) * t);

	// get all test cases
	for (i = 0; i < t; i++)
		for (j = 0; j < 5; j++)
		{
			for (k = 0; k < 5; k++)
				scanf("%d", &(states[i].m[j][k]));
			states[i].m[j][5] =
				states[i].m[j][0] << 20 ^
				states[i].m[j][1] << 15 ^
				states[i].m[j][2] << 10 ^
				states[i].m[j][3] << 5 ^
				states[i].m[j][4];
		}

	// start to find solutions
	for (i = 0; i < t; i++)
		state_test(i + 1, &states[i]);
}

int main(void)
{
	printf("[Computer Algorithm - 5*5 Puzzle] by team 3\n");
	puzzleSolution();
}

void solution_check()
{
	int i, j, move;
	int m[5][5] = { 0 };
	char c;
	bool fail;

	while (true)
	{
		printf("Input 0 to exit, else to continue.\n");
		scanf("%d", &i);
		if (i == 0) break;

		// init m and find position of the HOLE
		printf("Input the initial state.\n");
		for (i = 0; i < 5; i++)
			for (j = 0; j < 5; j++)
				scanf("%d", &m[i][j]);
		for (i = 24; i >= 0; i--)
			if (m[i / 5][i % 5] == HOLE)
			{
				j = i % 5;
				i = i / 5;
				break;
			}

		// handle exception
		if (i < 0)
		{
			printf("Invalid state! Try again.\n");
			continue;
		}
		
		// move according to the solution
		printf("Input the solution(ex: 4 L U U R).\n");
		scanf("%d", &move);
		while (move-- > 0)
		{
			scanf("%c", &c);
			switch (c)
			{
				case U:
					if (i == 0) printf("Can't move upward.\n");
					else
					{
						m[i][j] = m[i - 1][j];
						m[--i][j] = HOLE;
					}
					break;
				case D:
					if (i == 4) printf("Can't move downward.\n");
					else
					{
						m[i][j] = m[i + 1][j];
						m[++i][j] = HOLE;
					}
					break;
				case L:
					if (j == 0) printf("Can't move leftward.\n");
					else
					{
						m[i][j] = m[i][j - 1];
						m[i][--j] = HOLE;
					}
					break;
				case R:
					if (j == 4) printf("Can't move rightward.\n");
					else
					{
						m[i][j] = m[i][j + 1];
						m[i][++j] = HOLE;
					}
					break;
				default:
					move++;
					break;
			}
		}

		// check if the state is completed
		fail = false;
		for (i = 0; i < 5; i++)
		{
			for (j = 0; j < 5; j++)
			{
				if (m[i][j] != i * 5 + j + 1) fail = true;
				printf("%2d ", m[i][j]);
			}
			printf("\n");
		}
		printf(fail ? "fail\n\n" : "success\n\n");
	}
}
