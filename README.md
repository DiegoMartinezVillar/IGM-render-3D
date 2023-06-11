# Práctica Render 3D IGM

## Integrantes:
---
- Diego Martínez Villar - diego.martinez.villar@udc.es
- Lucas José Lara García - lucas.lara@udc.es
- Julio Fernández Fernández - julio.ffernandez@udc.es

### Requisitos Raytracing: 
---
- numpy
- matplotlib

### Ejecución OpenGL:
---
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

Para cambiar la camara empleada en OpenGL (quinta parte), se puede presionar la tecla `C` para cambiar entre dos cámaras distintas.

### Ejecución Raytracing:
---
Hay que moverse al directorio Raytracing:
```bash
cd RayTracing
```

Y ejecutarlo con:
```bash
python raytracing.py COMMAND

Commands:
  0           Uses the front view for rendering
  1           Uses the top view for rendering
```

## Versiones (tags):
---
Las versiones de la práctica se encuentran en las siguientes tags:

Para Raytracing:
- primeira_parte
- segunda_parte
- terceira_parte

Para OpenGL:
- opengl_primera_parte
- opengl_segunda_parte
- opengl_tercera_parte
- opengl_cuarta_parte
- opengl_quinta_parte

Estas etiquetas se corresponden con las partes de la práctica que se han ido realizando. En Raytracing, la etiqueta `tercera parte` se realizó antes que la `segunda parte`, porque al no haber conflicto entre ellas, se decidió realizarla antes.

La versión más actualizada del repositorio contiene la versión final de la práctica, a la que se le han añadido las distintas capturas de cada una de las partes. Estas capturas están disponibles en las carpetas `Capturas` de ambos directorios:

- OpenGL: [Capturas](OpenGL/Capturas)
- Raytracing: [Capturas](RayTracing/Capturas)
