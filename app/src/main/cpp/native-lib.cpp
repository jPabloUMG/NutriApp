#include <jni.h>
#include <string>
#include "database.h"
#include "calculos.h"

extern "C" {

// ══════════════════════════════════════════════════════════════════
//  MAIN ACTIVITY
// ══════════════════════════════════════════════════════════════════

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_MainActivity_inicializarDB(
        JNIEnv* env, jobject thiz, jstring ruta) {
    const char* rutaChar = env->GetStringUTFChars(ruta, nullptr);
    bool result = inicializarDB(std::string(rutaChar));
    env->ReleaseStringUTFChars(ruta, rutaChar);
    return result;
}

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_MainActivity_insertarUsuario(
        JNIEnv* env, jobject thiz,
        jstring nombre, jint edad, jstring genero,
        jstring departamento, jstring municipio,
        jfloat peso, jfloat altura,
        jstring actividad, jstring ocupacion, jstring meta) {

    Usuario u;
    u.nombre       = env->GetStringUTFChars(nombre, nullptr);
    u.edad         = edad;
    u.genero       = env->GetStringUTFChars(genero, nullptr);
    u.departamento = env->GetStringUTFChars(departamento, nullptr);
    u.municipio    = env->GetStringUTFChars(municipio, nullptr);
    u.peso         = peso;
    u.altura       = altura;
    u.actividad    = env->GetStringUTFChars(actividad, nullptr);
    u.ocupacion    = env->GetStringUTFChars(ocupacion, nullptr);
    u.meta         = env->GetStringUTFChars(meta, nullptr);
    return insertarUsuario(u);
}

JNIEXPORT jfloat JNICALL
Java_com_nutriapp_android_MainActivity_calcularIMC(
        JNIEnv* env, jobject thiz,
        jfloat peso, jfloat altura) {
    return calcularIMC(peso, altura);
}

JNIEXPORT jfloat JNICALL
Java_com_nutriapp_android_MainActivity_calcularTMB(
        JNIEnv* env, jobject thiz,
        jfloat peso, jfloat altura,
        jint edad, jstring genero) {
    const char* generoChar = env->GetStringUTFChars(genero, nullptr);
    float result = calcularTMB(peso, altura, edad, std::string(generoChar));
    env->ReleaseStringUTFChars(genero, generoChar);
    return result;
}

JNIEXPORT jfloat JNICALL
Java_com_nutriapp_android_MainActivity_calcularCaloriasRecomendadas(
        JNIEnv* env, jobject thiz,
        jfloat tmb, jstring actividad) {
    const char* actividadChar = env->GetStringUTFChars(actividad, nullptr);
    float result = calcularCaloriasRecomendadas(tmb, std::string(actividadChar));
    env->ReleaseStringUTFChars(actividad, actividadChar);
    return result;
}

JNIEXPORT jstring JNICALL
Java_com_nutriapp_android_MainActivity_generarRecomendaciones(
        JNIEnv* env, jobject thiz,
        jfloat peso, jfloat altura, jint edad,
        jstring genero, jstring actividad, jstring meta,
        jfloat calorias, jfloat carbs,
        jfloat proteinas, jfloat grasas, jfloat azucares) {

    Usuario u;
    u.peso      = peso;
    u.altura    = altura;
    u.edad      = edad;
    u.genero    = env->GetStringUTFChars(genero, nullptr);
    u.actividad = env->GetStringUTFChars(actividad, nullptr);
    u.meta      = env->GetStringUTFChars(meta, nullptr);

    ResultadoNutricional resultado = analizarNutricion(
            u, calorias, carbs, proteinas, grasas, azucares);
    std::string rec = generarRecomendaciones(resultado);
    return env->NewStringUTF(rec.c_str());
}

// Obtener perfil (genero, nombre)
JNIEXPORT jobjectArray JNICALL
Java_com_nutriapp_android_MainActivity_obtenerPerfilUsuario(
        JNIEnv* env, jobject thiz, jint usuarioId) {

    Perfil p = obtenerPerfil((int)usuarioId);
    jobjectArray arr = env->NewObjectArray(2,
                                           env->FindClass("java/lang/String"),
                                           env->NewStringUTF(""));
    env->SetObjectArrayElement(arr, 0, env->NewStringUTF(p.genero.c_str()));
    env->SetObjectArrayElement(arr, 1, env->NewStringUTF(p.nombre.c_str()));
    return arr;
}

// Obtener última medición extendida (14 campos)
JNIEXPORT jobjectArray JNICALL
Java_com_nutriapp_android_MainActivity_obtenerUltimaMedicion(
        JNIEnv* env, jobject thiz, jint usuarioId) {

    Medicion m = obtenerUltimaMedicion((int)usuarioId);
    jobjectArray arr = env->NewObjectArray(14,
                                           env->FindClass("java/lang/String"),
                                           env->NewStringUTF("-1"));

    if (m.id > 0) {
        env->SetObjectArrayElement(arr, 0,  env->NewStringUTF(std::to_string(m.imc).substr(0,5).c_str()));
        env->SetObjectArrayElement(arr, 1,  env->NewStringUTF(std::to_string(m.peso_lb).substr(0,6).c_str()));
        env->SetObjectArrayElement(arr, 2,  env->NewStringUTF(std::to_string(m.porcentaje_grasa).substr(0,5).c_str()));
        env->SetObjectArrayElement(arr, 3,  env->NewStringUTF(std::to_string(m.calorias_objetivo).substr(0,7).c_str()));
        env->SetObjectArrayElement(arr, 4,  env->NewStringUTF(std::to_string(m.masa_magra_lb).substr(0,6).c_str()));
        env->SetObjectArrayElement(arr, 5,  env->NewStringUTF(std::to_string(m.masa_muscular_lb).substr(0,6).c_str()));
        env->SetObjectArrayElement(arr, 6,  env->NewStringUTF(std::to_string(m.masa_grasa_lb).substr(0,6).c_str()));
        env->SetObjectArrayElement(arr, 7,  env->NewStringUTF(std::to_string(m.masa_osea_lb).substr(0,6).c_str()));
        env->SetObjectArrayElement(arr, 8,  env->NewStringUTF(std::to_string(m.agua_corporal_pct).substr(0,5).c_str()));
        env->SetObjectArrayElement(arr, 9,  env->NewStringUTF(std::to_string(m.grasa_visceral).substr(0,5).c_str()));
        env->SetObjectArrayElement(arr, 10, env->NewStringUTF(std::to_string(m.edad_metabolica).c_str()));
        env->SetObjectArrayElement(arr, 11, env->NewStringUTF(std::to_string(m.puntuacion_salud).substr(0,5).c_str()));
        env->SetObjectArrayElement(arr, 12, env->NewStringUTF(m.tipo_cuerpo.c_str()));
        env->SetObjectArrayElement(arr, 13, env->NewStringUTF(std::to_string(m.tmb).substr(0,7).c_str()));
    }
    return arr;
}

JNIEXPORT jfloat JNICALL
Java_com_nutriapp_android_MainActivity_obtenerCaloriasDia(
        JNIEnv* env, jobject thiz, jint usuarioId, jstring fecha) {
    const char* fechaChar = env->GetStringUTFChars(fecha, nullptr);
    float result = obtenerCaloriasDia((int)usuarioId, std::string(fechaChar));
    env->ReleaseStringUTFChars(fecha, fechaChar);
    return result;
}

// ══════════════════════════════════════════════════════════════════
//  LOGIN ACTIVITY
// ══════════════════════════════════════════════════════════════════

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_LoginActivity_inicializarDB(
        JNIEnv* env, jobject thiz, jstring ruta) {
    const char* rutaChar = env->GetStringUTFChars(ruta, nullptr);
    bool result = inicializarDB(std::string(rutaChar));
    env->ReleaseStringUTFChars(ruta, rutaChar);
    return result;
}

JNIEXPORT jobjectArray JNICALL
Java_com_nutriapp_android_LoginActivity_verificarLogin(
        JNIEnv* env, jobject thiz,
        jstring correo, jstring contrasena) {

    const char* correoChar     = env->GetStringUTFChars(correo, nullptr);
    const char* contrasenaChar = env->GetStringUTFChars(contrasena, nullptr);

    Usuario resultado;
    bool found = verificarLogin(
            std::string(correoChar),
            std::string(contrasenaChar),
            resultado);

    env->ReleaseStringUTFChars(correo, correoChar);
    env->ReleaseStringUTFChars(contrasena, contrasenaChar);

    jobjectArray arr = env->NewObjectArray(2,
                                           env->FindClass("java/lang/String"),
                                           env->NewStringUTF(""));

    if (found) {
        env->SetObjectArrayElement(arr, 0,
                                   env->NewStringUTF(std::to_string(resultado.id).c_str()));
        env->SetObjectArrayElement(arr, 1,
                                   env->NewStringUTF(std::to_string(resultado.rol_id).c_str()));
    } else {
        env->SetObjectArrayElement(arr, 0, env->NewStringUTF("-1"));
        env->SetObjectArrayElement(arr, 1, env->NewStringUTF("-1"));
    }
    return arr;
}

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_LoginActivity_verificarPerfil(
        JNIEnv* env, jobject thiz, jint usuarioId) {
    Perfil p = obtenerPerfil(usuarioId);
    return p.usuario_id > 0;
}

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_LoginActivity_registrarLog(
        JNIEnv* env, jobject thiz,
        jint usuarioId, jstring accion,
        jstring detalle, jstring pantalla, jint exitoso) {

    const char* accionChar   = env->GetStringUTFChars(accion, nullptr);
    const char* detalleChar  = env->GetStringUTFChars(detalle, nullptr);
    const char* pantallaChar = env->GetStringUTFChars(pantalla, nullptr);

    Log l;
    l.usuario_id = usuarioId;
    l.accion     = std::string(accionChar);
    l.detalle    = std::string(detalleChar);
    l.pantalla   = std::string(pantallaChar);
    l.fecha_hora = "2026-01-01 00:00:00";
    l.exitoso    = exitoso;

    env->ReleaseStringUTFChars(accion, accionChar);
    env->ReleaseStringUTFChars(detalle, detalleChar);
    env->ReleaseStringUTFChars(pantalla, pantallaChar);
    return insertarLog(l);
}

// ══════════════════════════════════════════════════════════════════
//  REGISTRO ACTIVITY
// ══════════════════════════════════════════════════════════════════

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_RegistroActivity_inicializarDB(
        JNIEnv* env, jobject thiz, jstring ruta) {
    const char* rutaChar = env->GetStringUTFChars(ruta, nullptr);
    bool result = inicializarDB(std::string(rutaChar));
    env->ReleaseStringUTFChars(ruta, rutaChar);
    return result;
}

JNIEXPORT jint JNICALL
Java_com_nutriapp_android_RegistroActivity_registrarUsuario(
        JNIEnv* env, jobject thiz,
        jstring correo, jstring contrasena) {

    const char* correoChar     = env->GetStringUTFChars(correo, nullptr);
    const char* contrasenaChar = env->GetStringUTFChars(contrasena, nullptr);

    int nuevoId = registrarUsuario(
            std::string(correoChar),
            std::string(contrasenaChar));

    env->ReleaseStringUTFChars(correo, correoChar);
    env->ReleaseStringUTFChars(contrasena, contrasenaChar);
    return (jint)nuevoId;
}

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_RegistroActivity_registrarLog(
        JNIEnv* env, jobject thiz,
        jint usuarioId, jstring accion,
        jstring detalle, jstring pantalla, jint exitoso) {

    const char* accionChar   = env->GetStringUTFChars(accion, nullptr);
    const char* detalleChar  = env->GetStringUTFChars(detalle, nullptr);
    const char* pantallaChar = env->GetStringUTFChars(pantalla, nullptr);

    Log l;
    l.usuario_id = usuarioId;
    l.accion     = std::string(accionChar);
    l.detalle    = std::string(detalleChar);
    l.pantalla   = std::string(pantallaChar);
    l.fecha_hora = "2026-01-01 00:00:00";
    l.exitoso    = exitoso;

    env->ReleaseStringUTFChars(accion, accionChar);
    env->ReleaseStringUTFChars(detalle, detalleChar);
    env->ReleaseStringUTFChars(pantalla, pantallaChar);
    return insertarLog(l);
}

// ══════════════════════════════════════════════════════════════════
//  PERFIL ACTIVITY
// ══════════════════════════════════════════════════════════════════

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_PerfilActivity_inicializarDB(
        JNIEnv* env, jobject thiz, jstring ruta) {
    const char* rutaChar = env->GetStringUTFChars(ruta, nullptr);
    bool result = inicializarDB(std::string(rutaChar));
    env->ReleaseStringUTFChars(ruta, rutaChar);
    return result;
}

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_PerfilActivity_guardarPerfil(
        JNIEnv* env, jobject thiz,
        jint usuarioId, jstring nombre, jint edad,
        jstring genero, jfloat altura, jfloat peso, jfloat grasa) {

    const char* nombreChar = env->GetStringUTFChars(nombre, nullptr);
    const char* generoChar = env->GetStringUTFChars(genero, nullptr);

    Perfil p;
    p.usuario_id      = (int)usuarioId;
    p.nombre          = std::string(nombreChar);
    p.edad            = (int)edad;
    p.genero          = std::string(generoChar);
    p.altura_cm       = (float)altura;
    p.nivel_actividad = "";
    p.objetivo        = "";
    p.modo_pro        = 0;

    env->ReleaseStringUTFChars(nombre, nombreChar);
    env->ReleaseStringUTFChars(genero, generoChar);
    return (jboolean)guardarPerfil(p, (float)peso, (float)grasa);
}

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_PerfilActivity_registrarLog(
        JNIEnv* env, jobject thiz,
        jint usuarioId, jstring accion,
        jstring detalle, jstring pantalla, jint exitoso) {

    const char* accionChar   = env->GetStringUTFChars(accion, nullptr);
    const char* detalleChar  = env->GetStringUTFChars(detalle, nullptr);
    const char* pantallaChar = env->GetStringUTFChars(pantalla, nullptr);

    Log l;
    l.usuario_id = usuarioId;
    l.accion     = std::string(accionChar);
    l.detalle    = std::string(detalleChar);
    l.pantalla   = std::string(pantallaChar);
    l.fecha_hora = "2026-01-01 00:00:00";
    l.exitoso    = exitoso;

    env->ReleaseStringUTFChars(accion, accionChar);
    env->ReleaseStringUTFChars(detalle, detalleChar);
    env->ReleaseStringUTFChars(pantalla, pantallaChar);
    return insertarLog(l);
}

// ══════════════════════════════════════════════════════════════════
//  OBJETIVOS ACTIVITY
// ══════════════════════════════════════════════════════════════════

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_ObjetivosActivity_inicializarDB(
        JNIEnv* env, jobject thiz, jstring ruta) {
    const char* rutaChar = env->GetStringUTFChars(ruta, nullptr);
    bool result = inicializarDB(std::string(rutaChar));
    env->ReleaseStringUTFChars(ruta, rutaChar);
    return result;
}

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_ObjetivosActivity_guardarObjetivo(
        JNIEnv* env, jobject thiz,
        jint usuarioId, jstring objetivo, jstring nivelActividad) {

    const char* objetivoChar  = env->GetStringUTFChars(objetivo, nullptr);
    const char* actividadChar = env->GetStringUTFChars(nivelActividad, nullptr);

    Perfil p = obtenerPerfil((int)usuarioId);
    p.objetivo        = std::string(objetivoChar);
    p.nivel_actividad = std::string(actividadChar);

    env->ReleaseStringUTFChars(objetivo, objetivoChar);
    env->ReleaseStringUTFChars(nivelActividad, actividadChar);
    return (jboolean)actualizarPerfil(p);
}

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_ObjetivosActivity_registrarLog(
        JNIEnv* env, jobject thiz,
        jint usuarioId, jstring accion,
        jstring detalle, jstring pantalla, jint exitoso) {

    const char* accionChar   = env->GetStringUTFChars(accion, nullptr);
    const char* detalleChar  = env->GetStringUTFChars(detalle, nullptr);
    const char* pantallaChar = env->GetStringUTFChars(pantalla, nullptr);

    Log l;
    l.usuario_id = usuarioId;
    l.accion     = std::string(accionChar);
    l.detalle    = std::string(detalleChar);
    l.pantalla   = std::string(pantallaChar);
    l.fecha_hora = "2026-01-01 00:00:00";
    l.exitoso    = exitoso;

    env->ReleaseStringUTFChars(accion, accionChar);
    env->ReleaseStringUTFChars(detalle, detalleChar);
    env->ReleaseStringUTFChars(pantalla, pantallaChar);
    return insertarLog(l);
}

// ══════════════════════════════════════════════════════════════════
//  REGISTRO COMIDA ACTIVITY
// ══════════════════════════════════════════════════════════════════

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_RegistroComidaActivity_inicializarDB(
        JNIEnv* env, jobject thiz, jstring ruta) {
    const char* rutaChar = env->GetStringUTFChars(ruta, nullptr);
    bool result = inicializarDB(std::string(rutaChar));
    env->ReleaseStringUTFChars(ruta, rutaChar);
    return result;
}

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_RegistroComidaActivity_guardarRegistroComida(
        JNIEnv* env, jobject thiz,
        jint usuarioId, jstring nombre, jstring cantidad,
        jfloat calorias, jfloat proteinas, jfloat carbohidratos,
        jfloat grasas, jstring tiempoComida, jstring fuente) {

    const char* nombreChar   = env->GetStringUTFChars(nombre, nullptr);
    const char* cantidadChar = env->GetStringUTFChars(cantidad, nullptr);
    const char* tiempoChar   = env->GetStringUTFChars(tiempoComida, nullptr);
    const char* fuenteChar   = env->GetStringUTFChars(fuente, nullptr);

    RegistroComida r;
    r.usuario_id      = (int)usuarioId;
    r.nombre_alimento = std::string(nombreChar);
    r.cantidad        = std::string(cantidadChar);
    r.calorias        = (float)calorias;
    r.proteinas       = (float)proteinas;
    r.carbohidratos   = (float)carbohidratos;
    r.grasas          = (float)grasas;
    r.azucares        = 0.0f;
    r.tiempo_comida   = std::string(tiempoChar);
    r.fuente          = std::string(fuenteChar);
    r.imagen_path     = "";
    r.confirmado      = 1;
    r.fecha           = "";
    r.hora            = "";

    env->ReleaseStringUTFChars(nombre, nombreChar);
    env->ReleaseStringUTFChars(cantidad, cantidadChar);
    env->ReleaseStringUTFChars(tiempoComida, tiempoChar);
    env->ReleaseStringUTFChars(fuente, fuenteChar);
    return (jboolean)insertarRegistroComida(r);
}
// ══════════════════════════════════════════════════════════════════
//  ADMIN ACTIVITY
// ══════════════════════════════════════════════════════════════════

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_AdminActivity_inicializarDB(
        JNIEnv* env, jobject thiz, jstring ruta) {
    const char* rutaChar = env->GetStringUTFChars(ruta, nullptr);
    bool result = inicializarDB(std::string(rutaChar));
    env->ReleaseStringUTFChars(ruta, rutaChar);
    return result;
}

JNIEXPORT jstring JNICALL
Java_com_nutriapp_android_AdminActivity_obtenerTodosUsuariosStr(
        JNIEnv* env, jobject thiz) {
    std::vector<Usuario> lista = obtenerTodosUsuarios();
    std::string result;
    for (const auto& u : lista) {
        result += std::to_string(u.id) + "|"
                  + u.correo + "|"
                  + std::to_string(u.rol_id) + "|"
                  + std::to_string(u.activo) + "\n";
    }
    return env->NewStringUTF(result.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_nutriapp_android_AdminActivity_obtenerTodosLogsStr(
        JNIEnv* env, jobject thiz) {
    std::vector<Log> lista = obtenerTodosLogs();
    std::string result;
    for (const auto& l : lista) {
        result += std::to_string(l.usuario_id) + "|"
                  + l.accion + "|"
                  + l.detalle + "|"
                  + l.pantalla + "|"
                  + l.fecha_hora + "|"
                  + std::to_string(l.exitoso) + "\n";
    }
    return env->NewStringUTF(result.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_nutriapp_android_AdminActivity_obtenerEstadisticasAdmin(
        JNIEnv* env, jobject thiz) {
    std::vector<Usuario> usuarios = obtenerTodosUsuarios();
    std::vector<Log> logs = obtenerTodosLogs();

    int totalUsuarios   = (int)usuarios.size();
    int totalLogs       = (int)logs.size();
    int usuariosActivos = 0;
    for (const auto& u : usuarios)
        if (u.activo) usuariosActivos++;

    std::string result = std::to_string(totalUsuarios) + "|"
                         + std::to_string(totalLogs) + "|"
                         + std::to_string(usuariosActivos);
    return env->NewStringUTF(result.c_str());
}

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_AdminActivity_desactivarUsuarioAdmin(
        JNIEnv* env, jobject thiz, jint usuarioId) {
    return (jboolean)desactivarUsuario((int)usuarioId);
}

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_AdminActivity_activarUsuarioAdmin(
        JNIEnv* env, jobject thiz, jint usuarioId) {
    return (jboolean)activarUsuario((int)usuarioId);
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_nutriapp_android_ReportesActivity_obtenerResumenReportes(
        JNIEnv* env, jobject thiz, jint usuarioId) {

    // Obtener fecha actual
    time_t t = time(nullptr);
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", localtime(&t));
    std::string fechaHoy(buffer);

    // Consultar comidas del día
    std::vector<RegistroComida> lista = obtenerRegistrosComida(usuarioId, fechaHoy);

    std::string resumen;
    for (const auto& r : lista) {
        resumen += r.fecha + " " + r.hora + " - " + r.tiempo_comida + ": "
                   + r.nombre_alimento + " (" + std::to_string((int)r.calorias) + " kcal)\n";
    }

    if (resumen.empty()) {
        resumen = "No hay comidas registradas para hoy.";
    }

    return env->NewStringUTF(resumen.c_str());
}

} // extern "C"
