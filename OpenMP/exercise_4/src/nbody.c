#include "nbody.h"

void nbody(struct Body *bodies, int steps, int output_steps, int N, double G, double DT, double EPS)
{
	FILE *checkpoint = NULL;
	char buffer[1024];

	double t1, t2;

	for (int i = 0; i < steps; i++) {
		if (output_steps != 0 && (i + output_steps) % output_steps == 0) {
			snprintf(buffer, 1024, "%d.txt", i);
			checkpoint = fopen(buffer, "w");
		}

		t1 = omp_get_wtime();

                        double fx[N][N];
                        double fy[N][N];
                        double fz[N][N];

#pragma omp parallel for collapse(2)
		for (int m = 0; m < N; m++) {
			for (int o = 0; o < N; o++) {
			fx[m][o] = 0.0;
			fy[m][o] = 0.0;
			fz[m][o] = 0.0;
			}
		}

                        double dx;
                        double dy;
                        double dz;

                        double f;
                        double r;

                        double ax;
                        double ay;
                        double az;


#pragma omp parallel for collapse(2) private(dx,dy,dz,r,f)
for (int j = 0; j < N; j++) {
			for (int k = 0; k < N; k++) {
				if (j != k) {
					dx = bodies[j].position[0] - bodies[k].old_position[0];
					dy = bodies[j].position[1] - bodies[k].old_position[1];
					dz = bodies[j].position[2] - bodies[k].old_position[2];
					r = sqrt(dx * dx + dy * dy + dz * dz);
					f = -G * (bodies[j].mass * bodies[k].mass) / pow((r * r) + (EPS * EPS), 1.5);

					fx[j][k] += f * dx / r;
					fy[j][k] += f * dy / r;
					fz[j][k] += f * dz / r;
				}
			}
}

#pragma omp parallel for 
for (int j = 0; j < N; j++) {
	for (int k = 1; k < N; k++) {
		fx[j][0] += fx[j][k];
		fy[j][0] += fy[j][k];
		fz[j][0] += fz[j][k];
	}
}


#pragma omp parallel for private (ax, ay, az)
		for (int j = 0; j < N; j++) {

			ax = fx[j][0] / bodies[j].mass;
			ay = fy[j][0] / bodies[j].mass;
			az = fz[j][0] / bodies[j].mass;
		
			bodies[j].velocity[0] += ax * DT;
			bodies[j].velocity[1] += ay * DT;
			bodies[j].velocity[2] += az * DT;
		
			bodies[j].position[0] += bodies[j].velocity[0] * DT;
			bodies[j].position[1] += bodies[j].velocity[1] * DT;
			bodies[j].position[2] += bodies[j].velocity[2] * DT;
              }


#pragma omp parallel for collapse(2)
		for (int j = 0; j < N; j++) {
                        for (int l = 0; l < 2; l++) {
			bodies[j].old_position[l] = bodies[j].position[l];
//			bodies[j].old_position[0] = bodies[j].position[0];
//			bodies[j].old_position[1] = bodies[j].position[1];
//			bodies[j].old_position[2] = bodies[j].position[2];

//			if (checkpoint != NULL)
//				fprintf(checkpoint, "%d\t%f\t%f\t%f\n\n\n", j, bodies[j].position[0], bodies[j].position[1], bodies[j].position[2]);
		}
}
		t2 = omp_get_wtime();
	
	if (checkpoint != NULL) {
			fclose(checkpoint);
			checkpoint = NULL;
		}

		printf("step = %d, runtime: %f\n", i, t2 - t1);
	}
}

