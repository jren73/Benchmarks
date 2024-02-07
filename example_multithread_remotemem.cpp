//original code
#pragma omp for nowait
		for(j = 0; j < lastrow - firstrow + 1; j++){
			suml = 0.0;
			for(k = rowstr[j]; k < rowstr[j+1]; k++){
				suml += a[k]*p[colidx[k]];
			}
			q[j] = suml;
		}

//example modified code using remote memory
int tid;
double *a=NULL;
#pragma omp for nowait
{   
    tid = omp_get_thread_num();
    
    for(j = 0; j < lastrow - firstrow + 1; j++){
        suml = 0.0;
        k = rowstr[j];
        //remote memory API:  int myread(struct resources res, uint64_t remote_mem_addr, double local_buf, size_t size);
        int n_element = min((rowstr[j+1] - rowstr[j]), block_size/sizeof(double)) ;
        for(int iter = 0; iter < ceil((rowstr[j+1] - rowstr[j]) / n_element); iter++){
            myread(res, (a + rowstr[j]+n_element*iter)* sizeof(double), res->buf + tid * block_size, n_element * sizeof(double));
            if (poll_completion(res)) {
                fprintf(stderr, "poll completion failed\n");
            }
            for(; k < rowstr[j+1] && k< rowstr[j]+n_element*iter; k++){
                //suml += a[k]*p[colidx[k]];
                suml += res->buf[tid * block_size + (k-rowstr[j]-n_element*iter)]*p[colidx[k]];
            }
        }
        q[j] = suml;
    }
}
