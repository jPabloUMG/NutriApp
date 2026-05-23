#include "database.h"
#include "calculos.h"
#include <android/log.h>

#define LOG_TAG "NutriApp"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static sqlite3* db = nullptr;

// ══════════════════════════════════════════════════════════════════
//  INICIALIZACIÓN
// ══════════════════════════════════════════════════════════════════

bool inicializarDB(const std::string& ruta) {
    int result = sqlite3_open(ruta.c_str(), &db);
    if (result != SQLITE_OK) {
        LOGE("Error abriendo DB: %s", sqlite3_errmsg(db));
        return false;
    }

    const char* sql =
            "CREATE TABLE IF NOT EXISTS roles ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "nombre TEXT NOT NULL,"
            "descripcion TEXT);"

            "CREATE TABLE IF NOT EXISTS usuarios ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "correo TEXT UNIQUE NOT NULL,"
            "contrasena TEXT NOT NULL,"
            "rol_id INTEGER DEFAULT 2,"
            "activo INTEGER DEFAULT 1,"
            "fecha_registro TEXT,"
            "ultimo_acceso TEXT,"
            "FOREIGN KEY(rol_id) REFERENCES roles(id));"

            "CREATE TABLE IF NOT EXISTS perfil ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "usuario_id INTEGER UNIQUE,"
            "nombre TEXT,"
            "edad INTEGER,"
            "genero TEXT,"
            "altura_cm REAL,"
            "nivel_actividad TEXT,"
            "objetivo TEXT,"
            "modo_pro INTEGER DEFAULT 0,"
            "fecha_creacion TEXT,"
            "FOREIGN KEY(usuario_id) REFERENCES usuarios(id));"

            "CREATE TABLE IF NOT EXISTS mediciones ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "usuario_id INTEGER,"
            "fecha TEXT,"
            "peso_lb REAL,"
            "porcentaje_grasa REAL,"
            "imc REAL,"
            "tmb REAL,"
            "masa_grasa_lb REAL,"
            "masa_magra_lb REAL,"
            "masa_muscular_lb REAL,"
            "masa_osea_lb REAL,"
            "agua_corporal_pct REAL,"
            "grasa_visceral REAL,"
            "edad_metabolica INTEGER,"
            "puntuacion_salud REAL,"
            "tipo_cuerpo TEXT,"
            "calorias_objetivo REAL,"
            "FOREIGN KEY(usuario_id) REFERENCES usuarios(id));"

            "CREATE TABLE IF NOT EXISTS registros_comida ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "usuario_id INTEGER,"
            "fecha TEXT,"
            "hora TEXT,"
            "tiempo_comida TEXT,"
            "nombre_alimento TEXT,"
            "cantidad TEXT,"
            "calorias REAL,"
            "proteinas REAL,"
            "carbohidratos REAL,"
            "grasas REAL,"
            "azucares REAL,"
            "imagen_path TEXT,"
            "fuente TEXT,"
            "confirmado INTEGER DEFAULT 0,"
            "FOREIGN KEY(usuario_id) REFERENCES usuarios(id));"

            "CREATE TABLE IF NOT EXISTS logs ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "usuario_id INTEGER,"
            "accion TEXT,"
            "detalle TEXT,"
            "pantalla TEXT,"
            "fecha_hora TEXT,"
            "exitoso INTEGER DEFAULT 1,"
            "FOREIGN KEY(usuario_id) REFERENCES usuarios(id));"

            "CREATE TABLE IF NOT EXISTS sesiones ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "usuario_id INTEGER,"
            "token TEXT,"
            "fecha_inicio TEXT,"
            "fecha_expiracion TEXT,"
            "activa INTEGER DEFAULT 1,"
            "dispositivo TEXT,"
            "FOREIGN KEY(usuario_id) REFERENCES usuarios(id));"

            "CREATE TABLE IF NOT EXISTS reportes ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "usuario_id INTEGER,"
            "fecha_inicio TEXT,"
            "fecha_fin TEXT,"
            "calorias_promedio REAL,"
            "proteinas_promedio REAL,"
            "carbs_promedio REAL,"
            "grasas_promedio REAL,"
            "peso_inicial_lb REAL,"
            "peso_final_lb REAL,"
            "fecha_generado TEXT,"
            "FOREIGN KEY(usuario_id) REFERENCES usuarios(id));";

    char* errMsg = nullptr;
    result = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (result != SQLITE_OK) {
        LOGE("Error creando tablas: %s", errMsg);
        sqlite3_free(errMsg);
        return false;
    }

    return inicializarRoles();
}

void cerrarDB() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

// ══════════════════════════════════════════════════════════════════
//  ROLES
// ══════════════════════════════════════════════════════════════════

bool inicializarRoles() {
    const char* sql =
            "INSERT OR IGNORE INTO roles (id, nombre, descripcion) VALUES "
            "(1, 'superadmin', 'Administrador con acceso total'),"
            "(2, 'usuario', 'Usuario regular de la app');";

    char* errMsg = nullptr;
    int result = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (result != SQLITE_OK) {
        LOGE("Error insertando roles: %s", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

// ══════════════════════════════════════════════════════════════════
//  USUARIOS
// ══════════════════════════════════════════════════════════════════

bool insertarUsuario(const Usuario& u) {
    const char* sql =
            "INSERT INTO usuarios (correo, contrasena, rol_id, activo, fecha_registro) "
            "VALUES (?, ?, ?, 1, ?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, u.correo.c_str(),         -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, u.contrasena.c_str(),     -1, SQLITE_STATIC);
    sqlite3_bind_int (stmt, 3, u.rol_id);
    sqlite3_bind_text(stmt, 4, u.fecha_registro.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

int registrarUsuario(const std::string& correo, const std::string& contrasena) {
    const char* sqlCheck = "SELECT id FROM usuarios WHERE correo = ?;";
    sqlite3_stmt* stmtCheck;
    sqlite3_prepare_v2(db, sqlCheck, -1, &stmtCheck, nullptr);
    sqlite3_bind_text(stmtCheck, 1, correo.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmtCheck) == SQLITE_ROW) {
        sqlite3_finalize(stmtCheck);
        return -1;
    }
    sqlite3_finalize(stmtCheck);

    const char* sql =
            "INSERT INTO usuarios (correo, contrasena, rol_id, activo, fecha_registro) "
            "VALUES (?, ?, 2, 1, datetime('now'));";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, correo.c_str(),     -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, contrasena.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result == SQLITE_DONE)
        return (int)sqlite3_last_insert_rowid(db);
    return 0;
}

bool verificarLogin(const std::string& correo,
                    const std::string& contrasena,
                    Usuario& resultado) {
    const char* sql =
            "SELECT id, correo, contrasena, rol_id, activo FROM usuarios "
            "WHERE correo = ? AND contrasena = ? AND activo = 1;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, correo.c_str(),     -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, contrasena.c_str(), -1, SQLITE_STATIC);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        resultado.id         = sqlite3_column_int(stmt, 0);
        resultado.correo     = (const char*)sqlite3_column_text(stmt, 1);
        resultado.contrasena = (const char*)sqlite3_column_text(stmt, 2);
        resultado.rol_id     = sqlite3_column_int(stmt, 3);
        resultado.activo     = sqlite3_column_int(stmt, 4);
        found = true;
    }
    sqlite3_finalize(stmt);
    return found;
}

bool actualizarUltimoAcceso(int usuario_id, const std::string& fecha) {
    const char* sql = "UPDATE usuarios SET ultimo_acceso = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, fecha.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int (stmt, 2, usuario_id);
    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

bool desactivarUsuario(int usuario_id) {
    const char* sql = "UPDATE usuarios SET activo = 0 WHERE id = ?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, usuario_id);
    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

std::vector<Usuario> obtenerTodosUsuarios() {
    std::vector<Usuario> lista;
    const char* sql =
            "SELECT id, correo, rol_id, activo, fecha_registro, ultimo_acceso "
            "FROM usuarios;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Usuario u;
        u.id             = sqlite3_column_int(stmt, 0);
        u.correo         = (const char*)sqlite3_column_text(stmt, 1);
        u.rol_id         = sqlite3_column_int(stmt, 2);
        u.activo         = sqlite3_column_int(stmt, 3);
        u.fecha_registro = (const char*)sqlite3_column_text(stmt, 4);
        u.ultimo_acceso  = sqlite3_column_text(stmt, 5) ?
                           (const char*)sqlite3_column_text(stmt, 5) : "";
        lista.push_back(u);
    }
    sqlite3_finalize(stmt);
    return lista;
}

// ══════════════════════════════════════════════════════════════════
//  PERFIL
// ══════════════════════════════════════════════════════════════════

bool insertarPerfil(const Perfil& p) {
    const char* sql =
            "INSERT OR REPLACE INTO perfil "
            "(usuario_id, nombre, edad, genero, altura_cm, nivel_actividad, "
            "objetivo, modo_pro, fecha_creacion) "
            "VALUES (?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int   (stmt, 1, p.usuario_id);
    sqlite3_bind_text  (stmt, 2, p.nombre.c_str(),          -1, SQLITE_STATIC);
    sqlite3_bind_int   (stmt, 3, p.edad);
    sqlite3_bind_text  (stmt, 4, p.genero.c_str(),          -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 5, p.altura_cm);
    sqlite3_bind_text  (stmt, 6, p.nivel_actividad.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text  (stmt, 7, p.objetivo.c_str(),        -1, SQLITE_STATIC);
    sqlite3_bind_int   (stmt, 8, p.modo_pro);
    sqlite3_bind_text  (stmt, 9, p.fecha_creacion.c_str(),  -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

bool actualizarPerfil(const Perfil& p) {
    const char* sql =
            "UPDATE perfil SET nombre=?, edad=?, genero=?, altura_cm=?, "
            "nivel_actividad=?, objetivo=?, modo_pro=? "
            "WHERE usuario_id=?;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text  (stmt, 1, p.nombre.c_str(),          -1, SQLITE_STATIC);
    sqlite3_bind_int   (stmt, 2, p.edad);
    sqlite3_bind_text  (stmt, 3, p.genero.c_str(),          -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 4, p.altura_cm);
    sqlite3_bind_text  (stmt, 5, p.nivel_actividad.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text  (stmt, 6, p.objetivo.c_str(),        -1, SQLITE_STATIC);
    sqlite3_bind_int   (stmt, 7, p.modo_pro);
    sqlite3_bind_int   (stmt, 8, p.usuario_id);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

Perfil obtenerPerfil(int usuario_id) {
    Perfil p;
    p.usuario_id = 0;
    const char* sql =
            "SELECT id, usuario_id, nombre, edad, genero, altura_cm, "
            "nivel_actividad, objetivo, modo_pro FROM perfil WHERE usuario_id = ?;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, usuario_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        p.id              = sqlite3_column_int(stmt, 0);
        p.usuario_id      = sqlite3_column_int(stmt, 1);
        p.nombre          = sqlite3_column_text(stmt, 2) ?
                            (const char*)sqlite3_column_text(stmt, 2) : "";
        p.edad            = sqlite3_column_int(stmt, 3);
        p.genero          = sqlite3_column_text(stmt, 4) ?
                            (const char*)sqlite3_column_text(stmt, 4) : "";
        p.altura_cm       = (float)sqlite3_column_double(stmt, 5);
        p.nivel_actividad = sqlite3_column_text(stmt, 6) ?
                            (const char*)sqlite3_column_text(stmt, 6) : "";
        p.objetivo        = sqlite3_column_text(stmt, 7) ?
                            (const char*)sqlite3_column_text(stmt, 7) : "";
        p.modo_pro        = sqlite3_column_int(stmt, 8);
    }
    sqlite3_finalize(stmt);
    return p;
}

// ── FIX: peso_lb se pasa directo a calculos.cpp, que ya convierte internamente ──
bool guardarPerfil(const Perfil& p, float peso_lb, float porcentaje_grasa) {

    // 1. Insertar/reemplazar perfil
    const char* sqlPerfil =
            "INSERT OR REPLACE INTO perfil "
            "(usuario_id, nombre, edad, genero, altura_cm, nivel_actividad, "
            "objetivo, modo_pro, fecha_creacion) "
            "VALUES (?,?,?,?,?,?,?,0,datetime('now'));";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sqlPerfil, -1, &stmt, nullptr);
    sqlite3_bind_int   (stmt, 1, p.usuario_id);
    sqlite3_bind_text  (stmt, 2, p.nombre.c_str(),          -1, SQLITE_STATIC);
    sqlite3_bind_int   (stmt, 3, p.edad);
    sqlite3_bind_text  (stmt, 4, p.genero.c_str(),          -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 5, p.altura_cm);
    sqlite3_bind_text  (stmt, 6, p.nivel_actividad.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text  (stmt, 7, p.objetivo.c_str(),        -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (result != SQLITE_DONE) return false;

    // 2. Calcular métricas — calculos.cpp convierte lb→kg internamente
    float imc            = calcularIMC(peso_lb, p.altura_cm);
    float tmb            = calcularTMB(peso_lb, p.altura_cm, p.edad, p.genero);
    float masa_grasa_lb  = peso_lb * (porcentaje_grasa / 100.0f);
    float masa_magra_lb  = peso_lb - masa_grasa_lb;
    float masa_muscular  = masa_magra_lb * 0.85f;
    float masa_osea      = peso_lb * 0.15f;
    float agua_pct       = (p.genero == "Masculino") ? 60.0f : 55.0f;
    float grasa_visceral = porcentaje_grasa / 5.0f;
    int   edad_metabolica= p.edad;
    float puntuacion     = 100.0f - (porcentaje_grasa * 0.5f);
    std::string tipo     = (porcentaje_grasa < 15.0f) ? "Atletico" :
                           (porcentaje_grasa < 25.0f) ? "Normal"   : "Alto";
    float calorias_obj   = calcularCaloriasRecomendadas(tmb, p.nivel_actividad);

    // 3. Insertar primera medición
    const char* sqlMed =
            "INSERT INTO mediciones "
            "(usuario_id, fecha, peso_lb, porcentaje_grasa, imc, tmb, "
            "masa_grasa_lb, masa_magra_lb, masa_muscular_lb, masa_osea_lb, "
            "agua_corporal_pct, grasa_visceral, edad_metabolica, "
            "puntuacion_salud, tipo_cuerpo, calorias_objetivo) "
            "VALUES (?,datetime('now'),?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmtM;
    sqlite3_prepare_v2(db, sqlMed, -1, &stmtM, nullptr);
    sqlite3_bind_int   (stmtM,  1, p.usuario_id);
    sqlite3_bind_double(stmtM,  2, peso_lb);
    sqlite3_bind_double(stmtM,  3, porcentaje_grasa);
    sqlite3_bind_double(stmtM,  4, imc);
    sqlite3_bind_double(stmtM,  5, tmb);
    sqlite3_bind_double(stmtM,  6, masa_grasa_lb);
    sqlite3_bind_double(stmtM,  7, masa_magra_lb);
    sqlite3_bind_double(stmtM,  8, masa_muscular);
    sqlite3_bind_double(stmtM,  9, masa_osea);
    sqlite3_bind_double(stmtM, 10, agua_pct);
    sqlite3_bind_double(stmtM, 11, grasa_visceral);
    sqlite3_bind_int   (stmtM, 12, edad_metabolica);
    sqlite3_bind_double(stmtM, 13, puntuacion);
    sqlite3_bind_text  (stmtM, 14, tipo.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmtM, 15, calorias_obj);

    result = sqlite3_step(stmtM);
    sqlite3_finalize(stmtM);
    return result == SQLITE_DONE;
}

// ══════════════════════════════════════════════════════════════════
//  MEDICIONES
// ══════════════════════════════════════════════════════════════════

bool insertarMedicion(const Medicion& m) {
    const char* sql =
            "INSERT INTO mediciones "
            "(usuario_id, fecha, peso_lb, porcentaje_grasa, imc, tmb, "
            "masa_grasa_lb, masa_magra_lb, masa_muscular_lb, masa_osea_lb, "
            "agua_corporal_pct, grasa_visceral, edad_metabolica, "
            "puntuacion_salud, tipo_cuerpo, calorias_objetivo) "
            "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int   (stmt,  1, m.usuario_id);
    sqlite3_bind_text  (stmt,  2, m.fecha.c_str(),       -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt,  3, m.peso_lb);
    sqlite3_bind_double(stmt,  4, m.porcentaje_grasa);
    sqlite3_bind_double(stmt,  5, m.imc);
    sqlite3_bind_double(stmt,  6, m.tmb);
    sqlite3_bind_double(stmt,  7, m.masa_grasa_lb);
    sqlite3_bind_double(stmt,  8, m.masa_magra_lb);
    sqlite3_bind_double(stmt,  9, m.masa_muscular_lb);
    sqlite3_bind_double(stmt, 10, m.masa_osea_lb);
    sqlite3_bind_double(stmt, 11, m.agua_corporal_pct);
    sqlite3_bind_double(stmt, 12, m.grasa_visceral);
    sqlite3_bind_int   (stmt, 13, m.edad_metabolica);
    sqlite3_bind_double(stmt, 14, m.puntuacion_salud);
    sqlite3_bind_text  (stmt, 15, m.tipo_cuerpo.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 16, m.calorias_objetivo);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

std::vector<Medicion> obtenerMediciones(int usuario_id) {
    std::vector<Medicion> lista;
    const char* sql =
            "SELECT id, usuario_id, fecha, peso_lb, porcentaje_grasa, imc, tmb, "
            "masa_grasa_lb, masa_magra_lb, masa_muscular_lb, masa_osea_lb, "
            "agua_corporal_pct, grasa_visceral, edad_metabolica, "
            "puntuacion_salud, tipo_cuerpo, calorias_objetivo "
            "FROM mediciones WHERE usuario_id = ? ORDER BY fecha DESC;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, usuario_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Medicion m;
        m.id               = sqlite3_column_int(stmt, 0);
        m.usuario_id       = sqlite3_column_int(stmt, 1);
        m.fecha            = (const char*)sqlite3_column_text(stmt, 2);
        m.peso_lb          = (float)sqlite3_column_double(stmt, 3);
        m.porcentaje_grasa = (float)sqlite3_column_double(stmt, 4);
        m.imc              = (float)sqlite3_column_double(stmt, 5);
        m.tmb              = (float)sqlite3_column_double(stmt, 6);
        m.masa_grasa_lb    = (float)sqlite3_column_double(stmt, 7);
        m.masa_magra_lb    = (float)sqlite3_column_double(stmt, 8);
        m.masa_muscular_lb = (float)sqlite3_column_double(stmt, 9);
        m.masa_osea_lb     = (float)sqlite3_column_double(stmt, 10);
        m.agua_corporal_pct= (float)sqlite3_column_double(stmt, 11);
        m.grasa_visceral   = (float)sqlite3_column_double(stmt, 12);
        m.edad_metabolica  = sqlite3_column_int(stmt, 13);
        m.puntuacion_salud = (float)sqlite3_column_double(stmt, 14);
        m.tipo_cuerpo      = sqlite3_column_text(stmt, 15) ?
                             (const char*)sqlite3_column_text(stmt, 15) : "";
        m.calorias_objetivo= (float)sqlite3_column_double(stmt, 16);
        lista.push_back(m);
    }
    sqlite3_finalize(stmt);
    return lista;
}

Medicion obtenerUltimaMedicion(int usuario_id) {
    Medicion m;
    m.id = 0;
    const char* sql =
            "SELECT id, usuario_id, fecha, peso_lb, porcentaje_grasa, imc, tmb, "
            "masa_grasa_lb, masa_magra_lb, masa_muscular_lb, masa_osea_lb, "
            "agua_corporal_pct, grasa_visceral, edad_metabolica, "
            "puntuacion_salud, tipo_cuerpo, calorias_objetivo "
            "FROM mediciones WHERE usuario_id = ? ORDER BY fecha DESC LIMIT 1;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, usuario_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        m.id               = sqlite3_column_int(stmt, 0);
        m.usuario_id       = sqlite3_column_int(stmt, 1);
        m.fecha            = (const char*)sqlite3_column_text(stmt, 2);
        m.peso_lb          = (float)sqlite3_column_double(stmt, 3);
        m.porcentaje_grasa = (float)sqlite3_column_double(stmt, 4);
        m.imc              = (float)sqlite3_column_double(stmt, 5);
        m.tmb              = (float)sqlite3_column_double(stmt, 6);
        m.masa_grasa_lb    = (float)sqlite3_column_double(stmt, 7);
        m.masa_magra_lb    = (float)sqlite3_column_double(stmt, 8);
        m.masa_muscular_lb = (float)sqlite3_column_double(stmt, 9);
        m.masa_osea_lb     = (float)sqlite3_column_double(stmt, 10);
        m.agua_corporal_pct= (float)sqlite3_column_double(stmt, 11);
        m.grasa_visceral   = (float)sqlite3_column_double(stmt, 12);
        m.edad_metabolica  = sqlite3_column_int(stmt, 13);
        m.puntuacion_salud = (float)sqlite3_column_double(stmt, 14);
        m.tipo_cuerpo      = sqlite3_column_text(stmt, 15) ?
                             (const char*)sqlite3_column_text(stmt, 15) : "";
        m.calorias_objetivo= (float)sqlite3_column_double(stmt, 16);
    }
    sqlite3_finalize(stmt);
    return m;
}

// ══════════════════════════════════════════════════════════════════
//  REGISTROS DE COMIDA
// ══════════════════════════════════════════════════════════════════

bool insertarRegistroComida(const RegistroComida& r) {
    const char* sql =
            "INSERT INTO registros_comida "
            "(usuario_id, fecha, hora, tiempo_comida, nombre_alimento, cantidad, "
            "calorias, proteinas, carbohidratos, grasas, azucares, "
            "imagen_path, fuente, confirmado) "
            "VALUES (?,date('now','localtime'),time('now','localtime'),?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int   (stmt,  1, r.usuario_id);
    sqlite3_bind_text  (stmt,  2, r.tiempo_comida.c_str(),   -1, SQLITE_STATIC);
    sqlite3_bind_text  (stmt,  3, r.nombre_alimento.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text  (stmt,  4, r.cantidad.c_str(),        -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt,  5, r.calorias);
    sqlite3_bind_double(stmt,  6, r.proteinas);
    sqlite3_bind_double(stmt,  7, r.carbohidratos);
    sqlite3_bind_double(stmt,  8, r.grasas);
    sqlite3_bind_double(stmt,  9, r.azucares);
    sqlite3_bind_text  (stmt, 10, r.imagen_path.c_str(),     -1, SQLITE_STATIC);
    sqlite3_bind_text  (stmt, 11, r.fuente.c_str(),          -1, SQLITE_STATIC);
    sqlite3_bind_int   (stmt, 12, r.confirmado);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

std::vector<RegistroComida> obtenerRegistrosComida(int usuario_id,
                                                   const std::string& fecha) {
    std::vector<RegistroComida> lista;
    const char* sql =
            "SELECT id, usuario_id, fecha, hora, tiempo_comida, nombre_alimento, "
            "cantidad, calorias, proteinas, carbohidratos, grasas, azucares, "
            "imagen_path, fuente, confirmado "
            "FROM registros_comida WHERE usuario_id = ? AND fecha = ? "
            "ORDER BY hora ASC;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int (stmt, 1, usuario_id);
    sqlite3_bind_text(stmt, 2, fecha.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        RegistroComida r;
        r.id              = sqlite3_column_int(stmt, 0);
        r.usuario_id      = sqlite3_column_int(stmt, 1);
        r.fecha           = (const char*)sqlite3_column_text(stmt, 2);
        r.hora            = (const char*)sqlite3_column_text(stmt, 3);
        r.tiempo_comida   = (const char*)sqlite3_column_text(stmt, 4);
        r.nombre_alimento = (const char*)sqlite3_column_text(stmt, 5);
        r.cantidad        = (const char*)sqlite3_column_text(stmt, 6);
        r.calorias        = (float)sqlite3_column_double(stmt, 7);
        r.proteinas       = (float)sqlite3_column_double(stmt, 8);
        r.carbohidratos   = (float)sqlite3_column_double(stmt, 9);
        r.grasas          = (float)sqlite3_column_double(stmt, 10);
        r.azucares        = (float)sqlite3_column_double(stmt, 11);
        r.imagen_path     = sqlite3_column_text(stmt, 12) ?
                            (const char*)sqlite3_column_text(stmt, 12) : "";
        r.fuente          = sqlite3_column_text(stmt, 13) ?
                            (const char*)sqlite3_column_text(stmt, 13) : "";
        r.confirmado      = sqlite3_column_int(stmt, 14);
        lista.push_back(r);
    }
    sqlite3_finalize(stmt);
    return lista;
}

float obtenerCaloriasDia(int usuario_id, const std::string& fecha) {
    const char* sql =
            "SELECT SUM(calorias) FROM registros_comida "
            "WHERE usuario_id = ? AND fecha = ?;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int (stmt, 1, usuario_id);
    sqlite3_bind_text(stmt, 2, fecha.c_str(), -1, SQLITE_STATIC);

    float total = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        total = (float)sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);
    return total;
}

// ══════════════════════════════════════════════════════════════════
//  LOGS
// ══════════════════════════════════════════════════════════════════

bool insertarLog(const Log& l) {
    const char* sql =
            "INSERT INTO logs (usuario_id, accion, detalle, pantalla, fecha_hora, exitoso) "
            "VALUES (?,?,?,?,?,?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int (stmt, 1, l.usuario_id);
    sqlite3_bind_text(stmt, 2, l.accion.c_str(),     -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, l.detalle.c_str(),    -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, l.pantalla.c_str(),   -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, l.fecha_hora.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int (stmt, 6, l.exitoso);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

std::vector<Log> obtenerLogs(int usuario_id) {
    std::vector<Log> lista;
    const char* sql =
            "SELECT id, usuario_id, accion, detalle, pantalla, fecha_hora, exitoso "
            "FROM logs WHERE usuario_id = ? ORDER BY fecha_hora DESC;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, usuario_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Log l;
        l.id         = sqlite3_column_int(stmt, 0);
        l.usuario_id = sqlite3_column_int(stmt, 1);
        l.accion     = (const char*)sqlite3_column_text(stmt, 2);
        l.detalle    = (const char*)sqlite3_column_text(stmt, 3);
        l.pantalla   = (const char*)sqlite3_column_text(stmt, 4);
        l.fecha_hora = (const char*)sqlite3_column_text(stmt, 5);
        l.exitoso    = sqlite3_column_int(stmt, 6);
        lista.push_back(l);
    }
    sqlite3_finalize(stmt);
    return lista;
}

std::vector<Log> obtenerTodosLogs() {
    std::vector<Log> lista;
    const char* sql =
            "SELECT id, usuario_id, accion, detalle, pantalla, fecha_hora, exitoso "
            "FROM logs ORDER BY fecha_hora DESC;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Log l;
        l.id         = sqlite3_column_int(stmt, 0);
        l.usuario_id = sqlite3_column_int(stmt, 1);
        l.accion     = (const char*)sqlite3_column_text(stmt, 2);
        l.detalle    = (const char*)sqlite3_column_text(stmt, 3);
        l.pantalla   = (const char*)sqlite3_column_text(stmt, 4);
        l.fecha_hora = (const char*)sqlite3_column_text(stmt, 5);
        l.exitoso    = sqlite3_column_int(stmt, 6);
        lista.push_back(l);
    }
    sqlite3_finalize(stmt);
    return lista;
}

// ══════════════════════════════════════════════════════════════════
//  SESIONES
// ══════════════════════════════════════════════════════════════════

bool insertarSesion(const Sesion& s) {
    const char* sql =
            "INSERT INTO sesiones "
            "(usuario_id, token, fecha_inicio, fecha_expiracion, activa, dispositivo) "
            "VALUES (?,?,?,?,1,?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int (stmt, 1, s.usuario_id);
    sqlite3_bind_text(stmt, 2, s.token.c_str(),            -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, s.fecha_inicio.c_str(),     -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, s.fecha_expiracion.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, s.dispositivo.c_str(),      -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

bool cerrarSesion(const std::string& token) {
    const char* sql = "UPDATE sesiones SET activa = 0 WHERE token = ?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_STATIC);
    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

bool validarSesion(const std::string& token) {
    const char* sql =
            "SELECT id FROM sesiones WHERE token = ? AND activa = 1;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_STATIC);
    bool valid = sqlite3_step(stmt) == SQLITE_ROW;
    sqlite3_finalize(stmt);
    return valid;
}

// ══════════════════════════════════════════════════════════════════
//  REPORTES
// ══════════════════════════════════════════════════════════════════

bool insertarReporte(const Reporte& r) {
    const char* sql =
            "INSERT INTO reportes "
            "(usuario_id, fecha_inicio, fecha_fin, calorias_promedio, "
            "proteinas_promedio, carbs_promedio, grasas_promedio, "
            "peso_inicial_lb, peso_final_lb, fecha_generado) "
            "VALUES (?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int   (stmt,  1, r.usuario_id);
    sqlite3_bind_text  (stmt,  2, r.fecha_inicio.c_str(),   -1, SQLITE_STATIC);
    sqlite3_bind_text  (stmt,  3, r.fecha_fin.c_str(),      -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt,  4, r.calorias_promedio);
    sqlite3_bind_double(stmt,  5, r.proteinas_promedio);
    sqlite3_bind_double(stmt,  6, r.carbs_promedio);
    sqlite3_bind_double(stmt,  7, r.grasas_promedio);
    sqlite3_bind_double(stmt,  8, r.peso_inicial_lb);
    sqlite3_bind_double(stmt,  9, r.peso_final_lb);
    sqlite3_bind_text  (stmt, 10, r.fecha_generado.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

std::vector<Reporte> obtenerReportes(int usuario_id) {
    std::vector<Reporte> lista;
    const char* sql =
            "SELECT id, usuario_id, fecha_inicio, fecha_fin, calorias_promedio, "
            "proteinas_promedio, carbs_promedio, grasas_promedio, "
            "peso_inicial_lb, peso_final_lb, fecha_generado "
            "FROM reportes WHERE usuario_id = ? ORDER BY fecha_generado DESC;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, usuario_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Reporte r;
        r.id                 = sqlite3_column_int(stmt, 0);
        r.usuario_id         = sqlite3_column_int(stmt, 1);
        r.fecha_inicio       = (const char*)sqlite3_column_text(stmt, 2);
        r.fecha_fin          = (const char*)sqlite3_column_text(stmt, 3);
        r.calorias_promedio  = (float)sqlite3_column_double(stmt, 4);
        r.proteinas_promedio = (float)sqlite3_column_double(stmt, 5);
        r.carbs_promedio     = (float)sqlite3_column_double(stmt, 6);
        r.grasas_promedio    = (float)sqlite3_column_double(stmt, 7);
        r.peso_inicial_lb    = (float)sqlite3_column_double(stmt, 8);
        r.peso_final_lb      = (float)sqlite3_column_double(stmt, 9);
        r.fecha_generado     = (const char*)sqlite3_column_text(stmt, 10);
        lista.push_back(r);
    }
    sqlite3_finalize(stmt);
    return lista;
}

std::vector<Reporte> obtenerTodosReportes() {
    std::vector<Reporte> lista;
    const char* sql =
            "SELECT id, usuario_id, fecha_inicio, fecha_fin, calorias_promedio, "
            "proteinas_promedio, carbs_promedio, grasas_promedio, "
            "peso_inicial_lb, peso_final_lb, fecha_generado "
            "FROM reportes ORDER BY fecha_generado DESC;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Reporte r;
        r.id                 = sqlite3_column_int(stmt, 0);
        r.usuario_id         = sqlite3_column_int(stmt, 1);
        r.fecha_inicio       = (const char*)sqlite3_column_text(stmt, 2);
        r.fecha_fin          = (const char*)sqlite3_column_text(stmt, 3);
        r.calorias_promedio  = (float)sqlite3_column_double(stmt, 4);
        r.proteinas_promedio = (float)sqlite3_column_double(stmt, 5);
        r.carbs_promedio     = (float)sqlite3_column_double(stmt, 6);
        r.grasas_promedio    = (float)sqlite3_column_double(stmt, 7);
        r.peso_inicial_lb    = (float)sqlite3_column_double(stmt, 8);
        r.peso_final_lb      = (float)sqlite3_column_double(stmt, 9);
        r.fecha_generado     = (const char*)sqlite3_column_text(stmt, 10);
        lista.push_back(r);
    }
    sqlite3_finalize(stmt);
    return lista;
}