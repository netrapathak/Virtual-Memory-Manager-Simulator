#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED
struct node
{
    int page_no;
    struct node *next;
};


struct queue
{
    struct node *first_list;
    int elements;
};
void enque(struct queue *lst,int element)
{
	lst->elements++;
    struct node *temp,*temp1=NULL;


        if (lst->first_list == NULL)
        {
            temp=(struct node *)malloc(sizeof(struct node));
            temp->page_no=element;
			temp->next=NULL;
            lst->first_list=(struct node *)malloc(sizeof(struct node));
            lst->first_list=temp;
        }
        else
        {
            temp=(struct node *)malloc(sizeof(struct node));
            temp1=(struct node *)malloc(sizeof(struct node));
            temp1=lst->first_list;
            if (!(temp == NULL))
            {
            	temp->page_no=element;
                temp->next=NULL;
                while(temp1->next!=NULL)
                    temp1=temp1->next;
                temp1->next=temp;
            }
            else
            {
                printf("\nQueue is Full...");
            }
        }

}
int deque(struct queue *q)
{
	q->elements--;
    if(q->first_list==NULL)
        printf("Queue is UnderFlow");
    else if(q->first_list->next==NULL)
    {
    	int ret=q->first_list->page_no;
        q->first_list=NULL;
        return ret;
    }
    else
    {
        struct node *temp=NULL;
        temp=(struct node *)malloc(sizeof(struct node));
        temp=q->first_list;
    	int ret=q->first_list->page_no;
		q->first_list=temp->next;
        free(temp);
        temp=NULL;
        return ret;
    }

}
#endif // QUEUE1_H_INCLUDED
