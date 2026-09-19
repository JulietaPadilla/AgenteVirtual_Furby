#pragma once

enum class EstadoFurby {
    HUEVO,
    NORMAL,
    HAMBRE,
    SUCIO,
    ENFERMO,
    CANSADO,
    HAMBRE_CANSADO,
    HAMBRE_SUCIO,
    HAMBRE_CANSADO_SUCIO,
    MUERTO
};

const char* NombreEstado(EstadoFurby estado);