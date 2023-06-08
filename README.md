# Práctica Render 3D IGM

## Integrantes:
- Diego Martínez Villar - diego.martinez.villar@udc.es
- Lucas José Lara García - lucas.lara@udc.es
- Julio Fernández Fernández - julio.ffernandez@udc.es

### Requisitos Raytracing: 
- numpy
- matplotlib

### Ejecución OpenGL:
Hay que moverse al directorio OpenGL:
```bash
cd OpenGL
```

Posteriormente, compilar el programa con:
```bash
make
# o alternativamente con
gcc spinningcube_withlight_SKEL.cpp textfile.c -lGL -lGLEW -lglfw -lm -o spinningcube_withlight
```

Y ejecutarlo con:
```bash
./spinningcube_withlight
```

### Ejecución Raytracing:
Hay que moverse al directorio Raytracing:
```bash
cd RayTracing
```

Y ejecutarlo con:
```bash
python raytracing.py COMMAND

Commands:
  - 0           (Uses the front view for rendering)
  - 1           (Uses the top view for rendering)
