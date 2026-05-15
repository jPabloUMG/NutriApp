#include <jni.h>
#include <string>
#include "database.h"
#include "calculos.h"

extern "C" {

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
// Login
JNIEXPORT jobjectArray JNICALL
Java_com_nutriapp_android_LoginActivity_verificarLogin(
        JNIEnv* env, jobject thiz,
        jstring correo, jstring contrasena) {

    const char* correoChar = env->GetStringUTFChars(correo, nullptr);
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

// Verificar si usuario tiene perfil
JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_LoginActivity_verificarPerfil(
        JNIEnv* env, jobject thiz, jint usuarioId) {
    Perfil p = obtenerPerfil(usuarioId);
    return p.usuario_id > 0;
}

// Registrar log
JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_LoginActivity_registrarLog(
        JNIEnv* env, jobject thiz,
        jint usuarioId, jstring accion,
        jstring detalle, jstring pantalla, jint exitoso) {

    Log l;
    l.usuario_id = usuarioId;
    l.accion = env->GetStringUTFChars(accion, nullptr);
    l.detalle = env->GetStringUTFChars(detalle, nullptr);
    l.pantalla = env->GetStringUTFChars(pantalla, nullptr);
    l.fecha_hora = "2026-01-01 00:00:00";
    l.exitoso = exitoso;

    return insertarLog(l);
}

// Inicializar DB desde LoginActivity
JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_LoginActivity_inicializarDB(
        JNIEnv* env, jobject thiz, jstring ruta) {
    const char* rutaChar = env->GetStringUTFChars(ruta, nullptr);
    bool result = inicializarDB(std::string(rutaChar));
    env->ReleaseStringUTFChars(ruta, rutaChar);
    return result;
}
// ── REGISTRO ACTIVITY ──────────────────────────────────────────────

JNIEXPORT jint JNICALL
Java_com_nutriapp_android_RegistroActivity_registrarUsuario(
        JNIEnv* env, jobject thiz,
        jstring correo, jstring contrasena) {

    const char* correoChar    = env->GetStringUTFChars(correo, nullptr);
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

    Log l;
    l.usuario_id = usuarioId;
    l.accion     = env->GetStringUTFChars(accion, nullptr);
    l.detalle    = env->GetStringUTFChars(detalle, nullptr);
    l.pantalla   = env->GetStringUTFChars(pantalla, nullptr);
    l.fecha_hora = "2026-01-01 00:00:00";
    l.exitoso    = exitoso;
    return insertarLog(l);
}

// ── PERFIL ACTIVITY ────────────────────────────────────────────────

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

    env->ReleaseStringUTFChars(nombre, nombreChar);
    env->ReleaseStringUTFChars(genero, generoChar);

    bool ok = guardarPerfil(p, (float)peso, (float)grasa);
    return (jboolean)ok;
}

JNIEXPORT jboolean JNICALL
Java_com_nutriapp_android_PerfilActivity_registrarLog(
        JNIEnv* env, jobject thiz,
        jint usuarioId, jstring accion,
        jstring detalle, jstring pantalla, jint exitoso) {

    Log l;
    l.usuario_id = usuarioId;
    l.accion     = env->GetStringUTFChars(accion, nullptr);
    l.detalle    = env->GetStringUTFChars(detalle, nullptr);
    l.pantalla   = env->GetStringUTFChars(pantalla, nullptr);
    l.fecha_hora = "2026-01-01 00:00:00";
    l.exitoso    = exitoso;
    return insertarLog(l);
}
} // extern "C"