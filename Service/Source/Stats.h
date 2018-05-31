/* ================================== zMeter ===========================================================================
 * File:     Stats.h
 * Author:   Eduardo Viciana (30 de mayo de 2018)
 * ------------------------------------------------------------------------------------------------------------------ */    
class Stats {
  private :
    typedef struct {
      uint16_t    *Frequency;                 //Distribucion de frecuencias
      uint16_t    *Voltage;                   //Distribucion de tensiones
      uint16_t    *THD;                       //Cada posicion representa un rango desde 0% hasta 8% (y el ultimo otros valores)
      uint16_t    Harmonics[2];               //Numero de muestras que cumplen los limites de harmonicos / no cumplen
    } Stats_t;
    static const float FrequencyRanges[];   //Rangos de frecuencias para estadisticas
    static const float VoltageRanges[];     //Rangos de tensiones para estadisticas
    static const float THDRanges[];         //Rangos de distorsion harmonica para estadisticas
    static const float HarmonicsAllowed[];  //Rangos permitidos para cada harmonico
};