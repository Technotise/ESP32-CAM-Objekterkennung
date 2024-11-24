#ifndef MODEL_RUN_H
#define MODEL_RUN_H

extern const char* mount_point;

bool classify_image();

#ifdef __cplusplus
extern "C" {
#endif

int run_model();

#ifdef __cplusplus
}
#endif

#endif // MODEL_RUN_H
