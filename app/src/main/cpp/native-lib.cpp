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

} // extern "C"