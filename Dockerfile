#
# Production image
#
FROM alpine
ADD resources resources
ADD NanaoChanBot NanaoChanBot

EXPOSE 8080

CMD ["./NanaoChanBot"]