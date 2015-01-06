#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include <mpi.h>
using namespace std;

#include "raytrace.h"

 bool init(char* inputName, scene &myScene) 
 {
   int nbMat, nbSphere, nbLight;
   int i;
   ifstream sceneFile(inputName);
   if (!sceneFile)
     return  false;
   sceneFile >> myScene.sizex >> myScene.sizey;
   sceneFile >> nbMat >> nbSphere >> nbLight;
   myScene.matTab.resize(nbMat); 
   myScene.sphTab.resize(nbSphere); 
   myScene.lgtTab.resize(nbLight); 
   for (i=0; i < nbMat; i++) 
     sceneFile >> myScene.matTab[i];
   for (i=0; i < nbSphere; i++) 
     sceneFile >> myScene.sphTab[i];
   for (i=0; i < nbLight; i++)
     sceneFile >> myScene.lgtTab[i];
   return true;
 } 

 bool hitSphere(const ray &r, const sphere &s, float &t) 
 { 
   // intersection rayon/sphere 
   vecteur dist = s.pos - r.start; 
   float B = r.dir * dist;
   float D = B*B - dist * dist + s.size * s.size; 
   if (D < 0.0f) 
     return false; 
   float t0 = B - sqrtf(D); 
   float t1 = B + sqrtf(D);
   bool retvalue = false;  
   if ((t0 > 0.1f) && (t0 < t)) 
   {
     t = t0;
     retvalue = true; 
   } 
   if ((t1 > 0.1f) && (t1 < t)) 
   {
     t = t1; 
     retvalue = true; 
   }
   return retvalue; 
 }

 bool draw(char* outputName, scene &myScene) 
 {
   ofstream imageFile(outputName,ios_base::binary);
   if (!imageFile)
     return false; 
   // Ajout du header TGA
   imageFile.put(0).put(0);
   imageFile.put(2);        /* RGB non compresse */

   imageFile.put(0).put(0);
   imageFile.put(0).put(0);
   imageFile.put(0);

   imageFile.put(0).put(0); /* origine X */ 
   imageFile.put(0).put(0); /* origine Y */

   imageFile.put((myScene.sizex & 0x00FF)).put((myScene.sizex & 0xFF00) / 256);
   imageFile.put((myScene.sizey & 0x00FF)).put((myScene.sizey & 0xFF00) / 256);
   imageFile.put(24);       /* 24 bit bitmap */ 
   imageFile.put(0); 
   // fin du header TGA

   // balayage 
   for (int y = 0; y < myScene.sizey; ++y) { 
   for (int x = 0; x < myScene.sizex; ++x) {
     float red = 0, green = 0, blue = 0;
     float coef = 1.0f;
     int level = 0; 
     // lancer de rayon 
     ray viewRay = { {float(x), float(y), -10000.0f}, { 0.0f, 0.0f, 1.0f}};
     do 
     { 
       // recherche de l'intersection la plus proche
       float t = 20000.0f;
       int currentSphere= -1;

       for (unsigned int i = 0; i < myScene.sphTab.size(); ++i) 
       { 
         if (hitSphere(viewRay, myScene.sphTab[i], t)) 
         {
           currentSphere = i;
         }
       }

       if (currentSphere == -1)
         break;

       point newStart = viewRay.start + t * viewRay.dir; 
       // la normale au point d'intersection 
       vecteur n = newStart - myScene.sphTab[currentSphere].pos;
       float temp = n * n;
       if (temp == 0.0f) 
         break; 

       temp = 1.0f / sqrtf(temp); 
       n = temp * n; 
       
       material currentMat = myScene.matTab[myScene.sphTab[currentSphere].material]; 

       // calcul de la valeur d'éclairement au point 
       for (unsigned int j = 0; j < myScene.lgtTab.size(); ++j) {
         light current = myScene.lgtTab[j];
         vecteur dist = current.pos - newStart;
         if (n * dist <= 0.0f)
           continue;
         float t = sqrtf(dist * dist);
         if ( t <= 0.0f )
           continue;
         ray lightRay;
         lightRay.start = newStart;
         lightRay.dir = (1/t) * dist;
         // calcul des ombres 
         bool inShadow = false; 
         for (unsigned int i = 0; i < myScene.sphTab.size(); ++i) {
           if (hitSphere(lightRay, myScene.sphTab[i], t)) {
             inShadow = true;
             break;
           }
         }
         if (!inShadow) {
           // lambert
           float lambert = (lightRay.dir * n) * coef;
           red += lambert * current.red * currentMat.red;
           green += lambert * current.green * currentMat.green;
           blue += lambert * current.blue * currentMat.blue;
         }
       }
         
       // on itére sur la prochaine reflexion
       coef *= currentMat.reflection;
       float reflet = 2.0f * (viewRay.dir * n);
       viewRay.start = newStart;
       viewRay.dir = viewRay.dir - reflet * n;

       level++;
     } 
     while ((coef > 0.0f) && (level < 10));   

     imageFile.put((unsigned char)min(blue*255.0f,255.0f)).put((unsigned char)min(green*255.0f, 255.0f)).put((unsigned char)min(red*255.0f, 255.0f));
   }
   }
   return true;
 }

 int main(int argc, char* argv[]) {
   if  (argc < 3) {
    return -1;
   }
   scene myScene;
   int taille, rang, hostlen;

  char hostname[MPI_MAX_PROCESSOR_NAME]={};
  MPI_Status status;
  MPI_Init(& argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &taille);
  MPI_Comm_rank(MPI_COMM_WORLD, &rang);
  MPI_Get_processor_name(hostname, &hostlen);

  if (rang == 0) {
   if (!init(argv[1], myScene))
     return -1;
   if (!draw(argv[2], myScene))
     return -1;
   return 0;
  }
}