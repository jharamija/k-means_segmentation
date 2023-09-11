#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>


#define LINESIZE 20

// #define FILENAME "minions.ppm"
// #define NEW_IMG "RGB_minions10.ppm"

// #define FILENAME "./images/beach.ppm"
// #define NEW_IMG "./images/RGB_beach8.ppm"

// #define FILENAME "x0_5.ppm"
// #define NEW_IMG "RGB_x0_5.ppm"

// #define FILENAME "animalsGrayscale.ppm"
// #define NEW_IMG "RGB_animalsGrayscale10.ppm"

// #define FILENAME "house.ppm"
// #define NEW_IMG "RGB_house3.ppm"

// #define FILENAME "lionKingDetailed.ppm"
// #define NEW_IMG "RGB_lionKingDetailed20.ppm"

// #define FILENAME "lionKingSimple.ppm"
// #define NEW_IMG "RGB_lionKingSimple5.ppm"

#define FILENAME "./images/beach.ppm"
#define NEW_IMG "RGB_nature50.ppm"

// #define FILENAME "img.ppm"
// #define NEW_IMG "test1.ppm"
// #define CENTR_TEST_IMG1 "imgOut1.ppm"
// #define CENTR_TEST_IMG2 "imgOut2.ppm"

#define MIN_K 2             // starts at 2 because it's the smallest number of centroids that makes sense
#define MAX_K 10
#define FIRST_CENTR -1      // if == -1 -> random
#define K 3               // if K < 1 -> random k


typedef struct {
    unsigned char r, g, b;
} pxColours;

typedef struct{
    int height, width, maxval, *centr;
    double *dist;           // distance to nearest centroid
    pxColours *colour;
} ppmImage;


int readHeader(ppmImage *image, FILE **f);
int readBody(ppmImage *image, FILE **f);
int writeImg(ppmImage *image);

int calculateCentroids(ppmImage *image,pxColours *centroids, int k, double *totalDistPerNumOfCentroids, const int num_of_data_points);

double calcDpDistance(pxColours pixel, pxColours centroids);
int getNextCentr(ppmImage *image, double totalDist);                // returns index of DP selected as the new centroid
int assignCentr(pxColours pixel, pxColours *centroids, int k);
int calcElbowPoint(ppmImage *image, double *totalDistPerNumOfCentroids, const int num_of_data_points);
int clustering(ppmImage *image, pxColours *centroids, int k);

int KMeans(ppmImage *image, int k, const int num_of_data_points, double *totalDistPerNumOfCentroids);
int writeCentroids(ppmImage *image, pxColours *centroids, char* WRITEFILE);


int main(){

    srand(time(NULL));

    int i, k=0;
    double *totalDistPerNumOfCentroids = calloc(MAX_K, sizeof(double));

    ppmImage *image = malloc(sizeof(ppmImage));

    FILE *f;
    f = fopen(FILENAME, "rb");

    readHeader(image, &f);
    readBody(image, &f);

    const int num_of_data_points = image->height * image->width;
    image->centr = (int *) malloc(num_of_data_points * sizeof(int));
    image->dist = (double *) calloc(num_of_data_points, sizeof(double));            // inits all distances to 0
    if(K < 1){
    //  k-means++ (slightly more advanced way of choosing centroids)
        for(k = MIN_K; k <= MAX_K; ++k){
            pxColours *centroids = (pxColours*) malloc(MAX_K * sizeof(pxColours));      // exists only within the loop

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
// writeCentroids(image, centroids, NEW_IMG);
// printf("%f\n", totalDistPerNumOfCentroids[k - MIN_K]);
            free(centroids);

        }

    //  final k to be used for clustering:
        k = calcElbowPoint(image, totalDistPerNumOfCentroids, num_of_data_points);
    } else{
        k = K;
    }
    printf("k = %d\n", k);

    KMeans(image, k, num_of_data_points, totalDistPerNumOfCentroids);

    fclose(f);
    free(image->colour);
    free(image->centr);
    free(image->dist);
    free(image);

    return 0;
}


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

    //  PRINTING K-MEANS RESULT
        newPx->r = centroids[ image->centr[i] ].r;
        newPx->g = centroids[ image->centr[i] ].g;
        newPx->b = centroids[ image->centr[i] ].b;

        fwrite(newPx, 3, 1, fn);
    }

    fclose(fn);
    free(newPx);

    return 0;
}

int KMeans(ppmImage *image, int k, const int num_of_data_points, double *totalDistPerNumOfCentroids){

    pxColours *centroids = (pxColours*) malloc(k * sizeof(pxColours));      // exists only within function

//  k-means++
    calculateCentroids(image, centroids, k, totalDistPerNumOfCentroids, num_of_data_points);

    clustering(image, centroids, k);

    writeCentroids(image, centroids, NEW_IMG);

    free(centroids);

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

//  finding elbow point by calculating the biggest change in PVE
    for(i = 1; i <= MAX_K - MIN_K; ++i){

        tmpDifference = explainedVariance[i] - explainedVariance[i - 1];
        if(tmpDifference > maxDifference){
            maxDifference = tmpDifference;
            retval = i + MIN_K;                     // converting index to num of clusters
        }
    }

    free(explainedVariance);

    return retval;
}

int clustering(ppmImage *image, pxColours *centroids, int k){

    int i, j, m=0;
    int sumR, sumG, sumB, count, convergence = 1;
    const int num_of_data_points = image->height * image->width;

//  calc
    while(convergence != 0){                        // if there is no change conv doesn't increment -> loop breaks
        for(i = 0; i < num_of_data_points; ++i){
            image->centr[i] = assignCentr(image->colour[i], centroids, k);      // centroids are assigned as an int between 0 and k (NUM_OF_CLUSTERS), *centroids is an array
        }

    //  finding mean values
        convergence = 0;

        for(i = 0; i < k; ++i){
            sumR = 0;
            sumG = 0;
            sumB = 0;
            count = 0;

            for(j = 0; j < num_of_data_points; ++j){
                if(image->centr[j] == i){
                    sumR += image->colour[j].r;
                    sumG += image->colour[j].g;
                    sumB += image->colour[j].b;
                    count++;
                }
            }

            if(centroids[i].r != ( sumR / count ) || centroids[i].g != ( sumG / count ) || centroids[i].b != ( sumB / count ))  // if there is no change conv increments -> loop breaks
                convergence++;              // if it is != 0 it means there has been change

            centroids[i].r = sumR / count;
            centroids[i].g = sumG / count;
            centroids[i].b = sumB / count;

            m++;
            char buf[12];
            snprintf(buf, 12, "beachTest%d", m);

            writeCentroids(image, centroids, buf);
        }
    }

    return 0;
}

//  k-means++ (slightly more advanced way of choosing centroids)
int calculateCentroids(ppmImage *image,pxColours *centroids, int k, double *totalDistPerNumOfCentroids, const int num_of_data_points){

    int i, j, nextCentrIndex, initFirstCentr;

    for(i = 0; i < k; ++i){

        if(i == 0){
        //  choosing a rand value for init of first centr
            if(FIRST_CENTR == -1){
                initFirstCentr = rand() % num_of_data_points;
            } else{
                initFirstCentr = FIRST_CENTR;
            }
        
        //  manual init of first centr
            centroids[i] = image->colour[initFirstCentr];

            for(j = 0; j < num_of_data_points; ++j){
                image->centr[j] = 0;                                                // only 1 centr exists
                image->dist[j] = calcDpDistance(image->colour[j], centroids[i]);    // calc dist to the only centr
                totalDistPerNumOfCentroids[k - MIN_K] += image->dist[j];                // calc total dist for the first centr
            }

        } else{
        //  every following centr
            nextCentrIndex = getNextCentr(image, totalDistPerNumOfCentroids[k - MIN_K]);    // calculates next centr based of distances between DPs and existing centroids
            centroids[i] = image->colour[nextCentrIndex];                               // assigns value to new centr
            totalDistPerNumOfCentroids[k - MIN_K] = 0;                                      // calc total dist with the newest centr

        //  reassigning data points to new closest centroid
            for(j = 0; j < num_of_data_points; ++j){
                image->centr[j] = assignCentr(image->colour[j], centroids, k);                      // reassigns centr to DPs so that each DP is assigned the centr it is closest to
                image->dist[j] = calcDpDistance(image->colour[j], centroids[ image->centr[j] ]);    // calc new distances to closest centr
                totalDistPerNumOfCentroids[k - MIN_K] += image->dist[j];                                // calc new total dist
            }
        }
    }

    return 0;
}

int assignCentr(pxColours pixel, pxColours *centroids, int k){

    float lowestDist = 1000.0f, tmp = 0.0f;
    int i, retval = -1;

    for(i = 0; i < k; ++i){
        tmp = sqrt( (pixel.r - centroids[i].r) * (pixel.r - centroids[i].r) +
                    (pixel.g - centroids[i].g) * (pixel.g - centroids[i].g) +
                    (pixel.b - centroids[i].b) * (pixel.b - centroids[i].b) );
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

int getNextCentr(ppmImage *image, double totalDist){

    const int num_of_data_points = image->height * image->width;
    int i = 0;
    double randVal = ((double) rand() / RAND_MAX) * totalDist;
    double partialSum = 0.0;

//  adds distances together until random value is reached (larger distances are more likely to reach the random value) -> more consistent (and better?) results
    // for(i = 0; i < num_of_data_points; ++i){
    //     partialSum += image->dist[i];

    //     if(partialSum >= randVal){
    //         return i;
    //     }
    // }

    while(partialSum < randVal){
        partialSum += image->dist[i];
        ++i;
        if(i == num_of_data_points) i = 0;
    }

    return i;

    // printf("%f, %f, %f\n", randVal, partialSum, totalDist);

    // printf("ERROR: getNextCentr() -> retval -1\n");
    // return -1;
}

//  calculates distance to given cenroid
//  in the first iter all pixels/DPs will be assigned to the first centroid (before this function is called)
//  in every following iter it will check if the new centr is closer than the previous one
//  ONLY RETURNS TMP, COMPARISON IS IN MAIN
double calcDpDistance(pxColours pixel, pxColours centroids){

    double tmp;

    tmp = sqrt( (pixel.r - centroids.r) * (pixel.r - centroids.r) +
                (pixel.g - centroids.g) * (pixel.g - centroids.g) +
                (pixel.b - centroids.b) * (pixel.b - centroids.b) );

    return tmp;
}

int readHeader(ppmImage *image, FILE **f){

    char *line, *ptr, *tmp;
    size_t size = LINESIZE;
    image->width = 0;
    image->height = 0;
    image->maxval = 0;

    line = (char *) malloc(LINESIZE);                   // will be reallocated by 'getline()' if needed

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

//  write header
    fprintf(fn, "P6\n");
    fprintf(fn, "%d %d\n", image->width, image->height);
    fprintf(fn, "%d\n", image->maxval);

//  write body (binary)
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
