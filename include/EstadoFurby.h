#pragma once

enum class EstadoFurby {
    HUEVO,
    NORMAL,
    HAMBRE,
    SUCIO,
    ENFERMO,
    CON_SUENO,
    MUERTO
};

const char* NombreEstado(EstadoFurby estado);