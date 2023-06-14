# ASSOOFS
Implementación de la práctica final de Ampliación de Sistemas Operativos en el grado en Ingeniería Informática de la Universidad de León, año 2023.
## Partes realizadas
He intentado hacer lo mejor que he podido:
- Parte obligatoria
- Caché de inodos
- Semáforos para ciertos recursos compartidos
- Borrado de ficheros (rm)
- Movimiento de ficheros (mv)
## Autor
Álvaro Prieto Álvarez (apriea04@estudiantes.unileon.es)
## Comentarios
Repositorio en GitHub: https://github.com/Apriea04/ASSOOFS
## Notas para el lector
- Esta práctica se ha modificado respecto a la versión entregada. Dicha versión está disponible en [este commit](https://github.com/Apriea04/ASSOOFS/tree/12a5e9b467c5cce836098ec84347ab9af24f24f4). Con la versión entragada obtuve una calificación de 10/10.
- En el repositorio se incluye el enunciado de esta práctica, propuesta por el profesor Ángel Manuel Guerrero Higueras. Ahí se especifica cómo se debe ejecutar esta práctica correctamente. Ejecutando [Preparacion.sh](https://github.com/Apriea04/ASSOOFS/blob/main/Preparacion.sh) con los **permisos adecuados** (sudo) se hacen parte de los pasos necesarios para poder probar la práctica.
- Existen una serie de comprobaciones que se deberían de llevar a cabo para evitar fallos, como pueden ser el uso y correcta verificación de los valores de retorno de funciones como [assoofs_get_a_freeblock](https://github.com/Apriea04/ASSOOFS/blob/main/assoofs.c#L60-L103) (donde se debería comprobar si se pudo encontrar un bloque libre o sino devolver error). Sin embargo, estas comprobaciones no fueron esenciales ni necesarias para obtener la máxima nota.
- Tras realizar pruebas extensas e intensivas se descubrió que la práctica no siempre funciona correctamente al asignar los bloques a los inodos. Otros compañeros que sólo implementaron la parte básica (código extraído del enunciado) realizaron las mismas pruebas y obtuvieron los mismos fallos. Consideramos en su momento que no eran extremadamente relevantes para una práctica de una asignatura como esta.
- En clase se nos especificó varias veces que esta práctica debía ser preparada para que funcionara en **Ubuntu 22.04 LTS.**.