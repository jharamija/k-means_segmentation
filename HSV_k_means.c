#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>


#define LINESIZE 20

// #define FILENAME "beach.ppm"
// #define NEW_IMG "HSVtest.ppm"

// #define FILENAME "x0_5.ppm"
// #define NEW_IMG "HSV_x0_5.ppm"

// #define FILENAME "animalsGrayscale.ppm"
// #define NEW_IMG "HSV_animalsGrayscale10.ppm"

// #define FILENAME "house.ppm"
// #define NEW_IMG "HSV_house10.ppm"

// #define FILENAME "lionKingDetailed.ppm"
// #define NEW_IMG "HSV_lionKingDetailed20.ppm"

// #define FILENAME "lionKingSimple.ppm"
// #define NEW_IMG "HSV_lionKingSimple10.ppm"

#define FILENAME "nature.ppm"
#define NEW_IMG "HSV_nature20.ppm"

#define MIN_K 2     // starts at 2 because it's the smallest number of centroids that makes sense
#define MAX_K 8
#define FIRST_CENTR -1      // if == -1 -> random
#define K 20                 // if K < 1 -> random k


typedef struct {
    unsigned char r, g, b;
} pxColours;

typedef struct {
    double h, s, v;
} pxHSV;

typedef struct{
    int height, width, maxval, *centr;
    double *dist;    // distance to nearest centroid
    pxColours *colour;
    pxHSV *colourHSV;
} ppmImage;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int readHeader(ppmImage *image, FILE **f){

    char *line, *ptr, *tmp;
    size_t size = LINESIZE;
    image->width = 0;
    image->height = 0;
    image->maxval = 0;

    line = (char *) malloc(LINESIZE);   // will be reallocated by 'getline()' if needed

    do{
        getline(&line, &size, *f);
        if(line[0] == '#' || *line == '\n'){            // skipping comments
            continue;
        } else if(line[0] == 'P'){                      // checking the magic number (necessary ?)
            line[1] == '6' ? 1
                           : printf("This is not a P6 .ppm file\n");
        } else if(!image->width && !image->height){     // getting image width and height
            image->width = strtol(line, &ptr, 10);
            tmp = ptr;
            image->height = strtol(tmp, &ptr, 10);
        } else if(!image->maxval){                      // getting image maxValue
            image->maxval = strtol(line, &ptr, 10);
        }
    } while(!image->maxval);

    free(line);

    return 0;
}

int readBody(ppmImage *image, FILE **f){

    // allocate memory for binary data
    image->colour = (pxColours*) malloc(image->width * image->height * sizeof(pxColours));

    // read body (binary)
    fread(image->colour, 3 * image->width, image->height, *f);

    return 0;
}

int writeImg(ppmImage *image){

    pxColours *newPx;
    newPx = (pxColours*) malloc(sizeof(pxColours));

    FILE *fn;
    fn = fopen(NEW_IMG, "wb");
    
    // write header
    fprintf(fn, "P6\n");
    fprintf(fn, "%d %d\n", image->width, image->height);
    fprintf(fn, "%d\n", image->maxval);

    // write body (binary)
    for(int i = 0; i < image->width * image->height; ++i){
        newPx->r = image->colour[i].r;
        newPx->g = image->colour[i].g;
        newPx->b = image->colour[i].b;

        fwrite(newPx, 3, 1, fn);
    }

    fclose(fn);
    free(newPx);

    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

double maxVal(double r, double g, double b){

    if(r > g && r > b){
        return r;
    } else if(g > b){
        return g;
    }

    return b;
}

double minVal(double r, double g, double b){

    if(r < g && r < b){
        return r;
    } else if(g < b){
        return g;
    }

    return b;
}

int rgbToHsv(ppmImage *image, const int num_of_data_points){

    image->colourHSV = (pxHSV*) malloc(num_of_data_points * sizeof(pxHSV));

    int i;
    double cmax, cmin, diff, rNorm, gNorm, bNorm;
    double rr, gg, bb;

    for(i = 0; i < num_of_data_points; ++i){

    //  normalizing r, g, b values to [0, 1]
        rr = 0;
        gg = 0;
        bb = 0;
        rr += image->colour[i].r;
        gg += image->colour[i].g;
        bb += image->colour[i].b;
        rNorm = rr / 255;
        gNorm = gg / 255;
        bNorm = bb / 255;

    //  finding max and min r, g, b values that will be used for further calculations
        cmax = maxVal(rNorm, gNorm, bNorm);
        cmin = minVal(rNorm, gNorm, bNorm);
        diff = cmax - cmin;

    //  v (value) is equal to the max value between r, g, b
        image->colourHSV[i].v = cmax * 100;     // [0, 100]

    //  s (saturation) is equal to the difference between max and min divided by max
        if(cmax == 0){
            image->colourHSV[i].s = 0;
        } else{
            image->colourHSV[i].s = (diff / cmax) * 100;    // [0, 100]
        }

    //  h (hue) depends on wether the max value is r, g or b
        if(diff == 0.0){
            image->colourHSV[i].h = 0;      // in this case h is undefined and can be any value since it doesn't impact the result
        } else if(cmax == rNorm){
            image->colourHSV[i].h = fmod(60 * ( (gNorm - bNorm) / diff ) + 360, 360.0);      // multiplied by 60 to convert it into degrees, modulo 360 to make it [0, 360]
        } else if(cmax == gNorm){
            image->colourHSV[i].h = fmod(60 * ( (bNorm - rNorm) / diff ) + 120, 360.0);
        } else{
            image->colourHSV[i].h = fmod(60 * ( (rNorm - gNorm) / diff ) + 240, 360.0);
        }
    }

    return 0;
}

// decAbs & decMod -> helping functions used to be able to use the absolute and modulo operators on decimal values
double decAbs(double x){

    if(x < 0) x *= -1;

    return x;
}

double decMod(double x, double y){

    while(x >= y){
        x -= y;
    }

    return x;
}

int hsvToRgb(pxHSV *centroids, pxColours *centroidsRGB, int k){

    int i;
    double hh, sNorm, vNorm, C, X, m, rr, gg, bb;   // C - chroma, X - intermediate value

    for(i = 0; i < k; ++i){

        hh = centroids[i].h;
    //  normalizing s, v values to [0, 1]
        sNorm = centroids[i].s / 100;
        vNorm = centroids[i].v / 100;

        if(sNorm == 0 || hh == 0){      // edge cases (pixels in shades of grey)
            rr = (int) vNorm * 255;
            gg = (int) vNorm * 255;
            bb = (int) vNorm * 255;

        } else{
            C = sNorm * vNorm;
            X = C * (1 - decAbs( decMod(hh / 60.0, 2.0) - 1 ) );
            m = vNorm - C;

            if(hh < 60){
                rr = C;
                gg = X;
                bb = 0;
            } else if(hh >= 60 && hh < 120){
                rr = X;
                gg = C;
                bb = 0;
            } else if(hh >= 120 && hh < 180){
                rr = 0;
                gg = C;
                bb = X;
            } else if(hh >= 180 && hh < 240){
                rr = 0;
                gg = X;
                bb = C;
            } else if(hh >= 240 && hh < 300){
                rr = X;
                gg = 0;
                bb = C;
            } else if(hh >= 300){
                rr = C;
                gg = 0;
                bb = X;
            }

            centroidsRGB[i].r = round( (rr + m) * 255 );
            centroidsRGB[i].g = round( (gg + m) * 255 );
            centroidsRGB[i].b = round( (bb + m) * 255 );
        }
    }

    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int writeCentroids(ppmImage *image, pxColours *centroids, char* WRITEFILE){

    pxColours *newPx;
    newPx = (pxColours*) malloc(sizeof(pxColours));

    FILE *fn;
    fn = fopen(WRITEFILE, "wb");
    
    // write header
    fprintf(fn, "P6\n");
    fprintf(fn, "%d %d\n", image->width, image->height);
    fprintf(fn, "%d\n", image->maxval);

    // write body (binary)
    for(int i = 0; i < image->width * image->height; ++i){

    //  PRINTING CLUSTER COLOURS (FOR 3 CLUSTERS)
        // if(i < 120000){
        //     newPx->r = centroids[0].r;
        //     newPx->g = centroids[0].g;
        //     newPx->b = centroids[0].b;
        // } else if(i < 240000){
        //     newPx->r = centroids[1].r;
        //     newPx->g = centroids[1].g;
        //     newPx->b = centroids[1].b;
        // } else{
        //     newPx->r = centroids[2].r;
        //     newPx->g = centroids[2].g;
        //     newPx->b = centroids[2].b;
        // }

    //  PRINTING K-MEANS RESULT
        newPx->r = centroids[ image->centr[i] ].r;
        newPx->g = centroids[ image->centr[i] ].g;
        newPx->b = centroids[ image->centr[i] ].b;

    //  PRINTING ORIGINAL IMAGE
        // newPx->r = image->colour[i].r;
        // newPx->g = image->colour[i].g;
        // newPx->b = image->colour[i].b;

        fwrite(newPx, 3, 1, fn);
    }

    fclose(fn);
    free(newPx);

    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

double calcDpDistance(pxHSV pixelHSV, pxHSV centroidsHSV){

    double tmp;

    tmp = sqrt( (pixelHSV.h - centroidsHSV.h) * (pixelHSV.h - centroidsHSV.h) +
                (pixelHSV.s - centroidsHSV.s) * (pixelHSV.s - centroidsHSV.s) +
                (pixelHSV.v - centroidsHSV.v) * (pixelHSV.v - centroidsHSV.v) );

    return tmp;
}

//  before (without totalDist): takes the furthest px as the new centr
//  now (with totalDist): randomly choosing a new centr with higher chances of choosing a px thats further from the existing centroids
int getNextCentr(ppmImage *image, const int num_of_data_points, double totalDist){

    // int i, retval = -1;
    // double max = 0.0;

    // for(i = 0; i < num_of_data_points+1000; ++i){
    //     if(image->dist[i] > max){
    //         max = image->dist[i];
    //         retval = i;
    //     }
    // }

    // return retval;

    int i = 0;
    double randVal = ((double) rand() / RAND_MAX) * totalDist;
    double partialSum = 0.0;

    while(partialSum < randVal){
        partialSum += image->dist[i];
        ++i;
        if(i == num_of_data_points) i = 0;
    }

    return i;
}

int assignCentr(pxHSV pixelHSV, pxHSV *centroidsHSV, int k){

    double lowestDist = 1000.0, tmp = 0.0;
    int i, retval = -1;

    for(i = 0; i < k; ++i){
        tmp = sqrt( (pixelHSV.h - centroidsHSV[i].h) * (pixelHSV.h - centroidsHSV[i].h) +
                    (pixelHSV.s - centroidsHSV[i].s) * (pixelHSV.s - centroidsHSV[i].s) +
                    (pixelHSV.v - centroidsHSV[i].v) * (pixelHSV.v - centroidsHSV[i].v) );
        if(tmp < lowestDist){
            lowestDist = tmp;
            retval = i;
        }
    }

    if(retval < 0 || retval > k){
        printf("Can't calculate closest centroid; retval = %d", retval);
    }


    return retval;
}

int calculateCentroids(ppmImage *image, pxHSV *centroidsHSV, int k, const int num_of_data_points, double *totalDistPerNumOfCentroids){

    int i, j, nextCentrIndex, initFirstCentr;

    for(i = 0; i < k; ++i){

        if(i == 0){
        //  choosing a rand value for init of first centr
            if(FIRST_CENTR == -1){
                initFirstCentr = ( rand() / RAND_MAX ) * num_of_data_points;
            } else{
                initFirstCentr = FIRST_CENTR;
            }
        
        //  manual init of first centr
            centroidsHSV[i] = image->colourHSV[initFirstCentr];

            for(j = 0; j <= num_of_data_points; ++j){
                image->centr[j] = 0;                                                        // only 1 centr exists
                image->dist[j] = calcDpDistance(image->colourHSV[j], centroidsHSV[i]);      // calc dist to the only centr
                totalDistPerNumOfCentroids[k - MIN_K] += image->dist[j];                        // calc total dist for the first centr
            }

        } else{
        //  every following centr
            nextCentrIndex = getNextCentr(image, num_of_data_points, totalDistPerNumOfCentroids[k - MIN_K]);        // calculates next centr based of distances between DPs and existing centroidsHSV
            centroidsHSV[i] = image->colourHSV[nextCentrIndex];                                                 // assigns value to new centr
            totalDistPerNumOfCentroids[k - MIN_K] = 0;                                                              // calc total dist with the newest centr

        //  reassigning data points to new closest centroid
            for(j = 0; j < num_of_data_points; ++j){
                image->centr[j] = assignCentr(image->colourHSV[j], centroidsHSV, k);                        // reassigns centr to DPs so that each DP is assigned the centr it is closest to
                image->dist[j] = calcDpDistance(image->colourHSV[j], centroidsHSV[ image->centr[j] ]);      // calc new distances to closest centr
                totalDistPerNumOfCentroids[k - MIN_K] += image->dist[j];                                        // calc new total dist
            }
        }
    }

    return 0;
}

int clustering(ppmImage *image, pxHSV *centroidsHSV, int k){

    int i, j;
    int sumH, sumS, sumV, count, convergence;
    const int num_of_data_points = image->height * image->width;

//  calc
    for(int n = 0; n < 1000; ++n){
        for(i = 0; i < num_of_data_points; ++i){
            image->centr[i] = assignCentr(image->colourHSV[i], centroidsHSV, k);      // centroidsHSV are assigned as an int between 0 and k (NUM_OF_CLUSTERS), *centroidsHSV is an array
        }

    //  finding mean values
        convergence = 0;

        for(i = 0; i < k; ++i){
            sumH = 0;
            sumS = 0;
            sumV = 0;
            count = 0;

            for(j = 0; j < num_of_data_points; ++j){
                if(image->centr[j] == i){
                    sumH += image->colourHSV[j].h;
                    sumS += image->colourHSV[j].s;
                    sumV += image->colourHSV[j].v;
                    count++;
                }
            }

            if(centroidsHSV[i].h != ( sumH / count ) || centroidsHSV[i].s != ( sumS / count ) || centroidsHSV[i].v != ( sumV / count ))  // if there is no change conv increments -> loop breaks
                convergence++;

            centroidsHSV[i].h = sumH / count;
            centroidsHSV[i].s = sumS / count;
            centroidsHSV[i].v = sumV / count;
        }

        if(convergence == 0)    // if there is no change conv increments -> loop breaks
            break;
    }

    return 0;
}

int getWCSS(ppmImage *image, pxHSV *centroidsHSV, int k, const int num_of_data_points, double *totalDistPerNumOfCentroids){

    int i;

//  elbow method -> finding optimal k
    clustering(image, centroidsHSV, k);

//  calculating WCSS (Within Cluster Sum of Squares) to be used for calculating the elbow point
    totalDistPerNumOfCentroids[k - MIN_K] = 0;
    for(i = 0; i < num_of_data_points; ++i){
        image->dist[i] = calcDpDistance(image->colourHSV[i], centroidsHSV[ image->centr[i] ]);
        totalDistPerNumOfCentroids[k - MIN_K] += image->dist[i];
    }

    return 0;
}

//  returns number of centr to be used for clustering
int calcElbowPoint(ppmImage *image, double *totalDistPerNumOfCentroids, const int num_of_data_points){

    int i, retval;
    double maxDifference = 0.0, tmpDifference = 0.0;

//  find a better way of calculating the elbow point than finding the biggest slope (PVE?)
//  totalDistPerNumOfCentroids[0] is total variance
    double *explainedVariance = (double *) malloc(MAX_K * sizeof(double));

//  calculating percentage of variance explained
    for(i = 0; i <= MAX_K - MIN_K; ++i){            // biggest index in totalDistPerNumOfCentroids is MAX_K - MIN_K
        explainedVariance[i] = ( ( totalDistPerNumOfCentroids[0] - totalDistPerNumOfCentroids[i] ) / totalDistPerNumOfCentroids[0] ) * 100;
    }

//  finding elbow point, by calculating the biggest change in PVE
    for(i = 1; i <= MAX_K - MIN_K; ++i){

        tmpDifference = explainedVariance[i] - explainedVariance[i - 1];
        if(tmpDifference > maxDifference){
            maxDifference = tmpDifference;
            retval = i + 2;                     // converting index to num of clusters
        }
    }

    free(explainedVariance);

    return retval;
}

int KMeansPP(ppmImage *image, int k, const int num_of_data_points, double *totalDistPerNumOfCentroids, char final){

//  image is transformed into HSV at this point, centroidsHSV is being used for calculations, CentroidsRGB is only used at the end to print results in RGB format
    pxHSV *centroidsHSV = (pxHSV*) malloc(k * sizeof(pxHSV));               // exists only within function
    pxColours *centroidsRGB = (pxColours*) malloc(k * sizeof(pxColours));   // exists only within function

    calculateCentroids(image, centroidsHSV, k, num_of_data_points, totalDistPerNumOfCentroids);

    clustering(image, centroidsHSV, k);

    getWCSS(image, centroidsHSV, k, num_of_data_points, totalDistPerNumOfCentroids);
    
    if(final == 'y'){
        hsvToRgb(centroidsHSV, centroidsRGB, k);

        writeCentroids(image, centroidsRGB, NEW_IMG);
    }

    free(centroidsHSV);
    free(centroidsRGB);

    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int main(){

    srand(time(NULL));

    int k;
    double *totalDistPerNumOfCentroids = calloc(MAX_K, sizeof(double));

    ppmImage *image = malloc(sizeof(ppmImage));

    FILE *f;
    f = fopen(FILENAME, "rb");

    readHeader(image, &f);
    readBody(image, &f);

    const int num_of_data_points = image->height * image->width;

    rgbToHsv(image, num_of_data_points);

    image->centr = (int *) malloc(num_of_data_points * sizeof(int));
    image->dist = (double *) calloc(num_of_data_points, sizeof(double));        // inits all distances to 0
    if(K < 1){
    //  k-means++ (slightly more advanced way of choosing centroids)
        for(k = MIN_K; k <= MAX_K; ++k){
            KMeansPP(image, k, num_of_data_points, totalDistPerNumOfCentroids, 'n');printf("k = %d\n", k);
        }

        k = calcElbowPoint(image, totalDistPerNumOfCentroids, num_of_data_points);
    } else{
        k = K;
    }
    printf("k = %d\n", k);
    KMeansPP(image, k, num_of_data_points, totalDistPerNumOfCentroids, 'y');


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
//  k-means++ (slightly more advanced way of choosing centroids)
    for(k = MIN_K; k <= MAX_K; ++k){

        pxColours *centroids = (pxColours*) malloc(MAX_K * sizeof(pxColours));      // exists only within function

    //  k-means++
        calculateCentroids(image, centroids, k, totalDistPerNumOfCentroids, num_of_data_points);

    //  elbow method -> finding optimal k
        clustering(image, centroids, k);

    //  calculating WCSS (Within Cluster Sum of Squares) to be used for calculating the elbow point
        totalDistPerNumOfCentroids[k - MIN_K] = 0;
        for(i = 0; i < num_of_data_points; ++i){
            image->dist[i] = calcDpDistance(image->colour[i], centroids[ image->centr[i] ]);
            totalDistPerNumOfCentroids[k - MIN_K] += image->dist[i];
        }

        // printf("%lf\n", totalDistPerNumOfCentroids[k - MIN_K]);

        // writeCentroids(image, centroids, "HSVttest.ppm");

        free(centroids);

    }

//  final k to be used for clustering:
    k = calcElbowPoint(image, totalDistPerNumOfCentroids, num_of_data_points);       // returns number of centr to be used for clustering
    
    // printf("%d\n", k);

    testFunct(image, k, num_of_data_points, totalDistPerNumOfCentroids);
*/
    fclose(f);
    free(image->colour);
    free(image->centr);
    free(image->dist);
    free(image);

    return 0;
}
