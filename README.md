# Cynthia's Zenith: A real-time indoor starry sky projector
## Key Features
* Scientific Accuracy: Real-time Gnomonic projection of stars up to magnitude 5.0.

* Customable Location: Automatic synchronization with local time via NTP to reflect the exact sky above you (currently optimized for Beijing/Haidian).

* Dynamic Events: Integrated meteor generation with realistic fading trails.

* Hardware: Custom high-power LED backlight modification for long-distance pr*ojection.

## Hardware Architecture
* The system utilizes a specialized 1.28" circular TFT display. To get better visual effects, the backlight was replaced by a more powerful one:

* Computation: ESP32-S3 handles the spherical trigonometry and Sprite-based rendering.

* Optics: A point-source 3W/5W LED coupled with a combination of lens (Fresnel/camera lens) to project the LCD image.

---

## **Mathematical Framework**

To map the celestial sphere onto a flat LCD screen without losing scientific integrity, the project implements the following mathematical pipeline:

### **1. 参数定义 (Parameter Definitions)**
* $\phi$ : Latitude
* $\lambda$ : Longitude
* $\delta$ : Declination
* $t$ : Hour Angle
* $R$ : Scaling Factor / Zenith Distance
* $c$ : Zenith Distance

### **2. Basic equations**
$$
\sin a = \sin \phi \sin \delta + \cos \phi \cos \delta \cos t
$$
1st equation (law of cosine in spherical surface)
$\tan A = \frac{\cos \delta \sin t}{\sin \phi \cos \delta \cos t - \cos \phi \sin \delta}$
or 
$$
\begin{cases} 
\cos a \sin A = -\cos \delta \sin t \\ 
\cos a \cos A = \cos \phi \sin \delta - \sin \phi \cos \delta \cos t 
\end{cases}
$$ 2nd eqaution
$$\sin A = \frac{-\cos \delta \sin t}{\cos a}$$ 3rd equation


### **3. Coordinate Transformation**
The system first converts **Equatorial coordinates** to **Horizontal coordinates** based on your local latitude ($\phi$) and the calculated **Local Sidereal Time** ($LST$). 
Let Z be the direction of Zenith, Y be the North, X be the East. 
Using the equations above: 
$$
\begin{cases} 
z= \cos c = \sin \phi \sin \delta + \cos \phi \cos \delta \cos t\\
x = \cos a \sin A = -\cos \delta \sin t\\ 
y = \cos a \cos A = \cos \phi \sin \delta - \sin \phi \cos \delta \cos t
\end{cases}
$$


### **4. Gnomonic Projection Formula**
The system uses gnominic projection to simulate the process of projection from a light source to the ceiling of the room. 
Using law of similar triangles: 

**Mapping to Screen**:
  - $$X =\frac {R \cdot x}{z} = R \cdot \frac{-\cos \delta \sin t}{\cos c}$$
  - $$Y =\frac{R \cdot y}{z} =R \cdot \frac{\cos \phi \sin \delta - \sin \phi \cos \delta \cos t}{\cos c}$$

> **Note**: A negative multiplier is applied to the X-axis to perform a "mirror Correction", ensuring the constellations match the perspective of a person looking upward at the ceiling.

---

## **Optical Calculations**

The clarity of your indoor starry sky is governed by the **Thin Lens Equation**. To project a sharp image across a room, the physical layout must be precisely calibrated:

### **The Focal Equation**
$$\frac{1}{f} = \frac{1}{u} + \frac{1}{v}$$
* **Object Distance ($u$)**: The distance between the LCD panel and the lens center.
* **Image Distance ($v$)**: The distance from the lens to your ceiling (typically 2.5m - 4.0m).

### **Magnification**
$$M=\frac{v}{u}$$

### **Indoor Optimization**
Since $v \gg f$, the object distance $u$ is set slightly beyond the focal length. This allows the small 1.28" screen to be magnified into a large-scale immersive sky while maintaining high star-point density.

---

## **Visual Rendering Logic**

### **Star Magnitude Hierarchy**
To mimic the human eye's perception of brightness, we use a **non-linear power curve**:
`Brightness = pow(normalized_mag, 1.5) * 255`
* **Tier 1**: Magnitudes < 1.5 (e.g., Sirius) are rendered as 2-pixel circles with a white core.
* **Tier 2**: Magnitudes < 3.0 are 1-pixel anti-aliased circles.
* **Tier 3**: Dimmer stars are rendered as single high-contrast pixels.

### **Meteor Dynamics**
Meteors are generated using a random vector algorithm. Each meteor possesses a **decaying life-cycle**, creating a fading trail effect by drawing progressively dimmer line segments in the direction of travel.
