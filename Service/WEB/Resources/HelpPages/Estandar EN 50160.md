La norma EN 50160 versa sobre las características de la tensión suministrada por las redes generales de distribución. Su aplicación se limita al punto de entrega de energía al cliente, tanto en baja tensión como en media tensión, y establece los límites o valores de las características de la tensión entregada por la red de distribución y esperables por el cliente. No se aplica a situaciones de explotación anormales como pueden ser averías, no conformidad con la instalación del cliente o condiciones excepcionales (entre otras).

## Objeto

El objeto principal de la norma es definir y describir los valores que caracterizan la tensión de alimentación suministrada, tales como:

* la frecuencia
* la amplitud
* la forma de la onda
* la simetría de las tensiones trifásicas

En explotación normal, estas características están sujetas a variaciones debidas a modificaciones de carga de la red, perturbaciones emitidas por ciertos equipos, o por la aparición de defectos debidos principalmente a causas externas. A causa de estas variaciones, los niveles de las características podrán ser sobrepasados alguna vez. Ciertos fenómenos que tienen una incidencia sobre la tensión son particularmente imprevisibles de tal modo que es imposible indicar el valor preciso de las características correspondientes. Conviene pues, interpretar en consecuencia los valores dados en esta norma para estos fenómenos, a saber: huecos de tensión e interrupciones de tensión.

## Sistemas en Baja Tensión
### Frecuencia

La frecuencia nominal de la tensión suministrada debe ser de 50 Hz para Europa. En condiciones normales de explotación, el valor medio de la frecuencia fundamental medida por períodos de 10 s debe situarse en los intervalos siguientes para redes acopladas por conexiones síncronas a un sistema interconectado::


<table style="border:1px solid black">
  <tr>
    <th>Límites</th>
    <th>Rango</th>
    <th>Escala temporal</th>
  </tr>
  <tr>
    <td><img src="http://latex.codecogs.com/gif.latex?\textup{50 Hz}\pm 1\%" /> </td>
    <td><img src="http://latex.codecogs.com/gif.latex?\textup{49.5 a 50.5 Hz}" /></td>
    <td><img src="http://latex.codecogs.com/gif.latex?\textup{Durante el 99.5\% de un a\~no}" /></td>
  </tr>
    <tr>
    <td><img src="http://latex.codecogs.com/gif.latex?$50 Hz \substack{+4.0\% \\ -6.0\%}" /> </td>
    <td><img src="http://latex.codecogs.com/gif.latex?\textup{47 a 52 Hz}" /></td>
    <td><img src="http://latex.codecogs.com/gif.latex?\textup{Durante el 100\% del tiempo}" /></td>
  </tr>
</table>

Actualmente no se realiza la implementación para redes sin conexión síncrona.

### Amplitud de tensión

El valor normalizado en Europa es de 400 V para la tensión de línea o compuesta (tensión entre fases) y de 230 V para la tensión de fase o simple (tensión entre fase y neutro).

### Variaciones de tensión

En las condiciones normales de explotación, aparte de las situaciones que siguen a defectos o a interrupciones:

<table style="border:1px solid black">
  <tr>
    <th>Límites</th>
    <th>Rango</th>
    <th>Escala temporal</th>
  </tr>
  <tr>
    <td><img src="http://latex.codecogs.com/gif.latex?\textup{230 V}\pm 10\%" /> </td>
    <td><img src="http://latex.codecogs.com/gif.latex?\textup{207 a 253 V}" /></td>
    <td rowspan="2" style='text-align:center;vertical-align:middle'><img src="http://latex.codecogs.com/gif.latex?\textup{Durante el 95\% de cada semana para los valores RMS promediados en intervalos de 10 minutos}" /></td>
  </tr>
  <tr>
    <td><img src="http://latex.codecogs.com/gif.latex?\textup{400 V}\pm 10\%" /> </td>
    <td><img src="http://latex.codecogs.com/gif.latex?\textup{360 a 440 V}" /></td>
  </tr>
    <tr>
    <td><img src="http://latex.codecogs.com/gif.latex?$230 V \substack{+10.0\% \\ -15.0\%}" /> </td>
    <td><img src="http://latex.codecogs.com/gif.latex?\textup{195.5 a 253 V}" /></td>
    <td rowspan="2" style='vertical-align:middle'><img src="http://latex.codecogs.com/gif.latex?\textup{Durante el 100\% de los intervalos de 10 minutos}" /></td>
  </tr>
 <tr>
    <td><img src="http://latex.codecogs.com/gif.latex?$400 V \substack{+10.0\% \\ -15.0\%}" /> </td>
    <td><img src="http://latex.codecogs.com/gif.latex?\textup{340 a 440 V}" /></td>
  </tr>
</table>

<strong>Nota</strong>: Según el RD 1955/2000, las variaciones de tensión en España para los consumidores finales no podrá ser superior al &plusmn;7% de la tensión nominal. 
El distribuidor estará obligado, con relación a cada uno de sus consumidores, a que el tiempo y número de interrupciones imprevistas mayores de tres minutos de cada año natural, dependiendo de la zona donde esté situado el suministro, no supere los siguientes valores:

<table style="border:1px solid black">
  <tbody><tr>
    <th>
      Zona
    </th>
    <th>
      Num. horas
    </th>
    <th>
      Num. interrupciones
    </th>
  </tr>
  <tr>
    <td>
      Zona urbana
    </td>
    <td style="text-align:center">
      5
    </td>
    <td style="text-align:center">
      10
    </td>
  </tr>
  <tr>
    <td>
      Zona semiurbana
    </td>
    <td style="text-align:center">
      9
    </td>
    <td style="text-align:center">
      13
    </td>
  </tr>
  <tr>
    <td>
      Zona rural concentrada
    </td>
    <td style="text-align:center">
      14
    </td>
    <td style="text-align:center">
      16
    </td>
  </tr>
  <tr>
    <td>
      Zona rural dispersa
    </td>
    <td style="text-align:center">
      19
    </td>
    <td style="text-align:center">
      22
    </td>
  </tr>
</tbody></table>

### Huecos de tensión

Se define como la disminución brusca de la tensión de la alimentación a un valor situado entre el 90% y el 1% de la tensión nominal, seguida del restablecimiento de la tensión después de un corto lapso de tiempo. Por convenio, un hueco de tensión dura entre 10 ms y 1 min. La profundidad de un hueco de tensión es definida como la diferencia entre la tensión eficaz mínima durante el hueco de tensión y la tensión declarada.

En condiciones normales de explotación, el número esperado de huecos de tensión en un año puede ir de algunas decenas a un millar. La mayor parte de los huecos de tensión tienen una duración de menos de un segundo y una profundidad inferior al 60%.

### Interrupciones breves de tensión

Una interrupción, en general, es una situación en la que la tensión en los puntos de suministro es inferior al 1% de la tensión nominal. Por lo general, su duración es inferior a 3 minutos. En condiciones normales de explotación, el número anual de interrupciones breves de la tensión puede variar de algunas decenas a varias centenas. La duración de aproximadamente el 70% de las interrupciones breves es inferior a 1 s.

### Interrupciones largas de tensión

En las condiciones normales de explotación, la frecuencia anual de las interrupciones de tensiones que sobrepasan 3 min puede ser inferior a 10 o alcanzar hasta 50, según las regiones.

### Desequilibrio de tensión

En condiciones normales de explotación, para cada período de una semana, el 95% de los valores eficaces promediados en 10 min de la componente inversa de la tensión de alimentación debe situarse entre el 0% y el 2% de la componente directa. Para poder hallar el desequilibrio, es necesario descomponer el sistema trifásico de alimentación en sus componentes directa, inversa y homopolar. El sistema oZm implementa la descomposición basada en el teorema de [Fortescue o componentes simétricas](https://es.wikipedia.org/wiki/Teorema_de_Fortescue). Para ampliar información sobre estos conceptos se puede consultar [este documento](Desbalanceo y potencia en sistemas trifásicos.pdf).

## Tensiones armónicas

En condiciones normales de explotación, durante cada período de una semana, el 95% de los valores eficaces de cada tensión armónica promediados en 10 min no debe sobrepasar los valores indicados en la tabla 1. Tensiones más elevadas para un armónico dado pueden ser debidas a resonancias.
Además, la tasa de distorsión armónica total de la tensión suministrada (THD) (comprendidos todos los armónicos hasta el orden 40) no debe sobrepasar el 8%.

<div class="centrado" style="text-align:center">
<table style="border:1px solid black; ">
<tr>
    <th colspan="4">Armónicos impares</th>
    <th colspan="2" rowspan="2" >Armónicos pares</th>
  </tr>
  <tr>
    <th colspan="2">No múltiplos de 3</td>
    <th colspan="2">Múltiplos de 3</td>
  </tr>
  <tr>
    <th>Orden h</td>
    <th>Tensión relativa</td>
    <th>Orden h</td>
    <th>Tensión relativa</td>
    <th>Orden h</td>
    <th>Tensión relativa</td>
  </tr>
  <tr>
    <td>5</td>
    <td>6</td>
    <td>3</td>
    <td>5</td>
    <td>2</td>
    <td>2</td>
  </tr>
  <tr>
    <td>7</td>
    <td>5</td>
    <td>9</td>
    <td>1.5</td>
    <td>4</td>
    <td>1</td>
  </tr>
  <tr>
    <td>11</td>
    <td>3.5</td>
    <td>15</td>
    <td>0.5</td>
    <td>6...24</td>
    <td>0.5</td>
  </tr>
  <tr>
    <td>13</td>
    <td>3</td>
    <td>21</td>
    <td>0.5</td>
    <td></td>
    <td></td>
  </tr>
  <tr>
    <td>17</td>
    <td>2</td>
    <td></td>
    <td></td>
    <td></td>
    <td></td>
  </tr>
  <tr>
    <td>19</td>
    <td>1.5</td>
    <td></td>
    <td></td>
    <td></td>
    <td></td>
  </tr>
  <tr>
    <td>23</td>
    <td>1.5</td>
    <td></td>
    <td></td>
    <td></td>
    <td></td>
  </tr>
  <tr>
    <td>25</td>
    <td>1.5</td>
    <td></td>
    <td></td>
    <td></td>
    <td></td>
  </tr>

</table>
</div>
