using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using AutoMapper;
using AutoMapperPerformanceTest.Model;
using AutoMapperPerformanceTest.ViewModel;
using Bogus;
using Xunit;

namespace AutoMapperPerformanceTest
{
    class Program
    {
        private static readonly Stopwatch StopWatch = new Stopwatch();


        static void Main(string[] args)
        {
            //tworzenie obiektów
            StopWatch.Start();
    
            var persons = new Faker<PersonEntity>()
                .RuleFor(x => x.Name, f => f.Name.FirstName())
                .RuleFor(x => x.Age, f => f.Random.Number(18, 65))
                .RuleFor(x => x.Salary, f => f.Finance.Amount(1000, 5000))
                .RuleFor(x => x.Office, f => new Office { Description = f.Lorem.Text(), OpeningHours = f.Date.Past() })
                .Generate(500000).ToList();

            Console.WriteLine("Tworzenie obiektów: " + StopWatch.Elapsed.TotalSeconds);

            //"ręczne przepisywanie"
            StopWatch.Restart();
            var personsManualGen = persons.Select(item => new PersonVm
            {
                Age = item.Age,
                Name = item.Name,
                OfficeDesc = item.Office.Description,
                RoundSalary = Convert.ToInt32(item.Salary)
            }).ToList();

            Console.WriteLine("Reczne przepisanie obiektow: " + StopWatch.Elapsed.TotalSeconds);

            //AutoMapper
            StopWatch.Restart();
            var config = new MapperConfiguration(cfg =>
            {
                cfg.CreateMap<PersonEntity, PersonVm>()
                    .ForMember(dest => dest.OfficeDesc, opt => opt.MapFrom(src => src.Office.Description))
                    .ForMember(dest => dest.RoundSalary, opt => opt.MapFrom(src => Convert.ToInt32(src.Salary)));
            });
            var mapper = config.CreateMapper();
            var personAutoMap = mapper.Map<List<PersonEntity>, List<PersonVm>>(persons);
            Console.WriteLine("AUTOMATYCZNE przepisanie obiektow: " + StopWatch.Elapsed.TotalSeconds);

            //testy
            StopWatch.Stop();

            Assert.True(personAutoMap.Count == persons.Count);

            Assert.NotEmpty(personsManualGen);
            Assert.NotEmpty(personAutoMap);

            Assert.All(personAutoMap, x => Assert.NotEmpty(x.Name));
            Assert.All(personAutoMap, x => Assert.NotEmpty(x.OfficeDesc));
            Assert.All(personAutoMap, x => Assert.NotStrictEqual(x.Age, 0));
            Assert.All(personAutoMap, x => Assert.NotStrictEqual(x.RoundSalary, 0));

            Assert.True(personsManualGen[persons.Count - 8].Name.Length > 1);
            Assert.True(personAutoMap[persons.Count - 8].Name.Length > 1);
            Assert.Same(personAutoMap[persons.Count - 8].Name, personAutoMap[persons.Count - 8].Name);

            Console.WriteLine("KONIEC!");
            Console.ReadKey();
        }
    }
}
