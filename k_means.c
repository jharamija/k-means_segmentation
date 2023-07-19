#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>


#define LINESIZE 20

#define FILENAME "beach.ppm"
#define NEW_IMG "test.ppm"
#define CENTR_TEST_IMG1 "centroids1.ppm"
#define CENTR_TEST_IMG2 "centroids2.ppm"

// #define FILENAME "img.ppm"
// #define NEW_IMG "test1.ppm"
// #define CENTR_TEST_IMG1 "imgOut1.ppm"
// #define CENTR_TEST_IMG2 "imgOut2.ppm"

#define MAX_K 5
#define NUM_OF_CLUSTERS 3
// #define FIRST_CENTR_INIT_PX 120     // initialization pixel for the first centroid

// let's just hard code it for now
// #define FIRST_CENTR -1      // random
#define FIRST_CENTR 1401
#define SECOND_CENTR 366850
#define THIRD_CENTR 367499

// #define FIRST_CENTR 650
// #define SECOND_CENTR 271000
// #define THIRD_CENTR 272639
// *******************************


typedef struct {
    unsigned char r, g, b;
} pxColours;

typedef struct{
    int height, width, maxval, *centr;
    double *dist;    // distance to nearest centroid
    pxColours *colour;
} ppmImage;


int readHeader(ppmImage *image, FILE **f);
int readBody(ppmImage *image, FILE **f);
int writeImg(ppmImage *image);

double calcDpDistance(pxColours pixel, pxColours centroids);
int getNextCentr(ppmImage *image, double totalDist);            // returns index of DP selected as the new centroid
int assignCentr(pxColours pixel, pxColours *centroids, int k);
int calcElbowPoint(ppmImage *image, double *totalDistPerNumOfCentroids, const int num_of_data_points);
int clustering(ppmImage *image, pxColours *centroids, int k);


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

int testFunct(ppmImage *image, int k, const int num_of_data_points, double *totalDistPerNumOfCentroids){

    int i, j, nextCentrIndex;

    pxColours *centroids = (pxColours*) malloc(MAX_K * sizeof(pxColours));      // exists only within function
//  put everything within this loop (from this point forwards) into a separate function?
    for(i = 0; i < k; ++i){

        if(i == 0){
        
        //  manual init of first centr
            // centroids[i].r = image->colour[initFirstCentr].r;
            // centroids[i].g = image->colour[initFirstCentr].g;
            // centroids[i].b = image->colour[initFirstCentr].b;
            centroids[i] = image->colour[FIRST_CENTR];

            for(j = 0; j < num_of_data_points; ++j){
                image->centr[j] = 0;        // only 1 centr exists

                image->dist[j] = calcDpDistance(image->colour[j], centroids[i]);
                totalDistPerNumOfCentroids[k - 2] += image->dist[j];
            }

            // printf("%lf\n", totalDistPerNumOfCentroids[0]);
        } else{

        //  every following centr
            nextCentrIndex = getNextCentr(image, totalDistPerNumOfCentroids[k - 2]);
            // centroids[i].r = image->colour[nextCentrIndex].r;
            // centroids[i].g = image->colour[nextCentrIndex].g;
            // centroids[i].b = image->colour[nextCentrIndex].b;
            centroids[i] = image->colour[nextCentrIndex];

        //  reassigning data points to new closest centroid
            totalDistPerNumOfCentroids[k - 2] = 0;

            for(j = 0; j < num_of_data_points; ++j){
                image->centr[j] = assignCentr(image->colour[j], centroids, k);
                image->dist[j] = calcDpDistance(image->colour[j], centroids[ image->centr[j] ]);
                totalDistPerNumOfCentroids[k - 2] += image->dist[j];
            }
        }
    }

    clustering(image, centroids, k);

    // printf("\n%lf\n", totalDistPerNumOfCentroids[k - 2]);
    writeCentroids(image, centroids, "ttest.ppm");

    return 0;
}

int main(){

    srand(time(NULL));

    int i, j, k, nextCentrIndex;
    double *totalDistPerNumOfCentroids = calloc(MAX_K, sizeof(double));

    ppmImage *image = malloc(sizeof(ppmImage));

    FILE *f;
    f = fopen(FILENAME, "rb");

    readHeader(image, &f);
    readBody(image, &f);

    const int num_of_data_points = image->height * image->width;
    image->centr = (int *) malloc(num_of_data_points * sizeof(int));
    image->dist = (double *) calloc(num_of_data_points, sizeof(double));        // inits all distances to 0
    // printf("%f\n", image->dist[200]);

    int initFirstCentr = FIRST_CENTR;

//  k-means++ (slightly more advanced way of choosing centroids)
    for(k = 2; k <= MAX_K; ++k){             // starts at 2 because it's the smallest number of centroids that makes sense

        pxColours *centroids = (pxColours*) malloc(MAX_K * sizeof(pxColours));      // exists only within function
//  put everything within this loop (from this point forwards) into a separate function?
        for(i = 0; i < k; ++i){

            if(i == 0){

                if(FIRST_CENTR == -1){
                    // implement rand init
                    initFirstCentr = rand() % num_of_data_points;
                }
            
            //  manual init of first centr
                // centroids[i].r = image->colour[initFirstCentr].r;
                // centroids[i].g = image->colour[initFirstCentr].g;
                // centroids[i].b = image->colour[initFirstCentr].b;
                centroids[i] = image->colour[initFirstCentr];

                for(j = 0; j < num_of_data_points; ++j){
                    image->centr[j] = 0;        // only 1 centr exists

                    image->dist[j] = calcDpDistance(image->colour[j], centroids[i]);
                    totalDistPerNumOfCentroids[k - 2] += image->dist[j];
                }

                // printf("%lf\n", totalDistPerNumOfCentroids[0]);
            } else{

            //  every following centr
                nextCentrIndex = getNextCentr(image, totalDistPerNumOfCentroids[k - 2]);
                // centroids[i].r = image->colour[nextCentrIndex].r;
                // centroids[i].g = image->colour[nextCentrIndex].g;
                // centroids[i].b = image->colour[nextCentrIndex].b;
                centroids[i] = image->colour[nextCentrIndex];

            //  reassigning data points to new closest centroid
                totalDistPerNumOfCentroids[k - 2] = 0;

                for(j = 0; j < num_of_data_points; ++j){
                    image->centr[j] = assignCentr(image->colour[j], centroids, k);
                    image->dist[j] = calcDpDistance(image->colour[j], centroids[ image->centr[j] ]);
                    totalDistPerNumOfCentroids[k - 2] += image->dist[j];
                }
            }
        }

    //  elbow method -> finding optimal k
        clustering(image, centroids, k);

    //  calculating WCSS (Within Cluster Sum of Squares) to be used for calculating the elbow point
        totalDistPerNumOfCentroids[k - 2] = 0;
        for(i = 0; i < num_of_data_points; ++i){
            image->dist[i] = calcDpDistance(image->colour[i], centroids[ image->centr[i] ]);
            totalDistPerNumOfCentroids[k - 2] += image->dist[i];
        }

        // printf("%lf\n", totalDistPerNumOfCentroids[k - 2]);

        // writeCentroids(image, centroids, "ttest.ppm");

        free(centroids);

    }

//  final k to be used for clustering:
    k = calcElbowPoint(image, totalDistPerNumOfCentroids, num_of_data_points);       // returns number of centr to be used for clustering
    
    // printf("\t%d\n", k);
    
    testFunct(image, k, num_of_data_points, totalDistPerNumOfCentroids);

    // writeImg(image);

    fclose(f);
    free(image->colour);
    free(image->centr);
    free(image->dist);
    free(image);

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
    for(i = 0; i <= MAX_K - 2; ++i){            // biggest index in totalDistPerNumOfCentroids is MAX_K - 2
        explainedVariance[i] = ( ( totalDistPerNumOfCentroids[0] - totalDistPerNumOfCentroids[i] ) / totalDistPerNumOfCentroids[0] ) * 100;
    }

//  finding elbow point, by calculating the biggest change in PVE
    for(i = 1; i <= MAX_K - 2; ++i){

        tmpDifference = explainedVariance[i] - explainedVariance[i - 1];
        if(tmpDifference > maxDifference){
            // printf("%d\t%lf\t%lf\n", i, tmpDifference, maxDifference);
            maxDifference = tmpDifference;
            retval = i + 2;     // converting index to num of clusters
        }
    }

    free(explainedVariance);

    return retval;
}

int clustering(ppmImage *image, pxColours *centroids, int k){

    int i, j;
    int sumR, sumG, sumB, count, convergence;
    const int num_of_data_points = image->height * image->width;

//  calc
    for(int n = 0; n < 1000; ++n){
        for(i = 0; i < num_of_data_points; ++i){
            image->centr[i] = assignCentr(image->colour[i], centroids, k);      // centroids are assigned as an int between 0 and k (NUM_OF_CLUSTERS), *centroids is an array
        }

//      finding mean values
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
                convergence++;

            centroids[i].r = sumR / count;
            centroids[i].g = sumG / count;
            centroids[i].b = sumB / count;
        }

        if(convergence == 0)    // if there is no change conv increments -> loop breaks
            break;
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

    // int i;
    // double randVal = ((double) rand() / RAND_MAX) * totalDist;
    // double partialSum = 0.0;
    const int num_of_data_points = image->height * image->width;

    // for(i = 0; i < num_of_data_points; ++i){     // adds distances together until random value is reached (larger distances are more likely to reach the random value)
    //     partialSum += image->dist[i];

    //     if(partialSum >= randVal){
    //         return i;
    //     }
    // }

    // return -1;

    int i, retval;
    double maxDist = 0.0;

    for(i = 0; i < num_of_data_points; ++i){        // always returns the furthest DP to be the new centr -> more consistent (and better?) results

        if(image->dist[i] >= maxDist){
            maxDist = image->dist[i];
            retval = i;
        }
    }

    return retval;
}

//  calculates distance to given cenroid
//  in the first iter all pixels/DPs will be assigned to the first centroid
//  in every following iter it will check if the new centr is closer than the previous one
//  returns the distance to the closest centr -> old dist if the new centr is further and new dist if its closer

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
