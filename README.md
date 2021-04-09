TP1 Sistemas Operativos 2 
### Ingeniería en Computación - FCEFyN - UNC
# Interprocess Communication

## Introducción
Los mecanismos de IPC le permite a los procesos intercambiar información. Hemos visto que se pueden clasificar en dos grandes grupos, los de transferencia de datos (pipe, FIFO, message queue, sockets, etc.) y los de memoria compartida (shared memory, memory mapping, etc.). Todo este conjunto de mecanismos son sumamente útiles en todo proyecto y es lo que motiva el presente trabajo.

## Objetivo
El objetivo del presente trabajo práctico es que el estudiante sea capaz de diseñar e implementar un software que haga uso de los mecanismos de IPC que provee el Sistema Operativo, implementando lo visto en el teórico, práctico y haciendo uso todos los conocimientos adquiridos en Ingeniería de Software y Sistemas Operativos I.

## Desarrollo
Se pide que diseñe, implemente y testee un software (desarrollado en lenguaje C), siguiendo el diagrama de la Figura 1, la cual describe un sistema utilizando el patrón [Publisher Suscriber][publisher].

![Diagrama de Componentes](/img/block_diagram.png)
*Figura 1: Diagrama de Componentes del sistema*

### Delivery Manager
Se debe implementar un proceso que se encargue de recibir mensajes y enviarlos a todos los suscriptores validos. A este proceso, lo vamos a  vamos a llamar _DeliveryManager_. El mismo debe contemplar lo siguiente:

-   Debe poseer una interfaz (CLI) que acepte unicamente los siguientes comandos:
    - `add <socket> <productor>` : Este comando agregara el socket a una lista correspondiente al servicio, para ser validado.
    - `delete <socket> <productor>`: Este comando borra el host como suscriptor dejando de enviarle mensajes.
    - `log <socket>`. Este comando comprime el log local del _DeliveryManager_ y lo envía a `<socket>`.
-	El _DeliveryManager_ debe validar que los hosts agregados sean aptos para recibir mensaje del tipo `<Mensaje>|<Checksum>`.	
-	Una vez validado el host, todos los mensajes que recibe el _DeliveryManager_ de un productor, deben ser enviados a los suscriptores correspondientes.
-	En caso que se desconecte un suscriptor, el servicio para los demás suscriptores no debe verse afectado. Luego de cinco segundos de mensajes fallados, debe ser eliminado de la lista. Si el suscriptor vuelve antes de esos 5 segundos, debe enviársele todos los mensajes en colados.
-	El _DeliveryManager_ debe loguear todos los mensajes enviados, tanto el origen como el destino, también se agrega o se elimina un suscriptor y cuando es validado. El formato del log debe `<datetime> <Mensaje>`. 


### Productores
Se deben implementar tres productores:
- Un productor que envía un mensaje random con una tasa de X/segundos.
- Un productor que envía la memoria libre del Sistema, cada Y/segundos.
- Un productor que debe enviar load del sistema normalizado, cada Z/segundos.
Pueden elegir otro productor, justificándolo. X, Y y Z deben ser distintos.


### Suscriptores
- Puede existir hasta cinco mil suscriptores, y deben ser capaz de vivir en una misma instancia.
- Los suscriptores deben esperar que el _DeliveryManager_ los suscriba mediante el comando `add`, es decir, que lo suscriba.
- Los suscriptores deben validar el `checksum` de los mensajes recibidos y loguear el mensaje, para luego ser descartados.


## Restricciones
El diseño debe contemplar toda situación no descripta en el presente documento y se debe hacer un correcto manejo de errores. 

## Criterios de Corrección
- Se debe compilar el código con los flags de compilación: 
     -Wall -Pedantic -Werror -Wextra -Wconversion -std=gnu11
- La correcta gestion de memoria.
- Dividir el código en módulos de manera juiciosa.
- Estilo de código.
- Manejo de errores
- El código no debe contener errores, ni warnings.
- El código no debe contener errores de cppcheck.

## Entrega
La entrega se hace a travéz del repositorio de GitHub y se deberá demostrar la realizacion del mismo mediante un correcto uso del [workflow de Git][workflow]. El repositorio deberá proveer los archivos fuente y cualquier otro archivo asociados a la compilación, archivos  de  proyecto  ”Makefile”  y  el  código correctamente documentado. No debe contener ningún archivo asociado a proyectos de un IDE y se debe asumir que el proyecto podrá ser compilado y corrido por una `tty` en una distribución de Linux con las herramientas típicas de desarrollo. También se deberá entregar un informe (que pude ser en MD en el repo) explicando paso a paso el desarrllo, inluyendo graficos del diseño solución propuesto, justificando en todo momento lo implementrado.

## Evaluación
El presente trabajo práctico es individual y deberá entregarse antes de las 23:50ART del día 22 de Abril de 2021 dejando asentado en el LEV con el archivo de ifnorme. Será corregido y luego se coordinará una fecha para la defensa oral del mismo.

## Links / Referencias
- [Patrón Publisher-Suscriber](https://en.wikipedia.org/wiki/Publish%E2%80%93subscribe_pattern)
- [Git Workflow](https://www.atlassian.com/git/tutorials/comparing-workflows)
- [The Linux Programming Interfaces: Chapter 43](https://github.com/rangaeeeee/books-raspberrypimagazines/blob/18bf13c133ef0a78b2b60c4ff8635ac71b0843f1/The%20Linux%20Programming%20Interface-Michael%20Kerrisk.pdf)

[publisher]: https://en.wikipedia.org/wiki/Publish%E2%80%93subscribe_pattern "Patrón Publisher-Suscriber"
[workflow]: https://www.atlassian.com/git/tutorials/comparing-workflows "Git Workflow"
